#include "compiler/codegen.h"
#include "module/instr.h"
#include "util/alloc.h"
#include "util/serial.h"
#include "util/error.h"
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>

#define ER_CINSTR_LABEL 0x100

typedef struct {
    er_opcode_t opc;
    uint16_t data;
} er_ginstr_t;

typedef struct {
    struct {
        er_const_t **tab;
        size_t size;
        size_t cap;
    } consts;

    struct {
        er_ginstr_t *code;
        size_t code_size;
        size_t code_cap;
    } func;
} er_genctx_t;

static void er_genctx_init(er_genctx_t *g) {
    g->consts.cap = 8;
    g->consts.size = 0;
    g->consts.tab = er_xmalloc(g->consts.cap * sizeof(er_const_t *));

    g->func.code_cap = 8;
    g->func.code_size = 0;
    g->func.code = er_xmalloc(g->func.code_cap * sizeof(er_ginstr_t));
}

static void er_genctx_destruct(er_genctx_t *g) {
    free(g->consts.tab);
    free(g->func.code);
}

static void er_genctx_clear_func(er_genctx_t *g) {
    g->func.code_size = 0;
}

static void er_genctx_print(er_genctx_t *g) {
    fprintf(stderr, "{\n");

    for (size_t i = 0; i < g->func.code_size; i++) {
        er_ginstr_t *instr = &g->func.code[i];

        if (instr->opc == ER_CINSTR_LABEL) {
            fprintf(stderr, "  .%" PRId16 "\n", 
                    instr->data);
        } else {
            int len = fprintf(stderr, "    %s ", er_opcode_name(instr->opc));

            for (int i = 0; i < 20 - len; i++) {
                fputc(' ', stderr);
            }

            fprintf(stderr, "%" PRId16 "\n", instr->data);
        }
    }

    fprintf(stderr, "}\n");
}

static void er_emit(er_genctx_t *g, er_opcode_t opc, uint16_t data) {
    if (g->func.code_size + 1 > g->func.code_cap) {
        g->func.code_cap *= 2;
        g->func.code = er_xrealloc(g->func.code, 
                                   g->func.code_cap * sizeof(er_ginstr_t));
    }

    er_ginstr_t *instr = &g->func.code[g->func.code_size];
    g->func.code_size++;

    instr->opc = opc;
    instr->data = data;
}

static uint16_t er_add_const(er_genctx_t *g, void *c, 
                             er_consttag_t tag, int size) {
    if (g->consts.cap + 1 > g->consts.size) {
        g->consts.cap *= 2;
        g->consts.tab = er_xrealloc(g->consts.tab, 
                                    g->consts.cap * sizeof(er_const_t *));
    }

    er_const_t *cc = er_xmalloc(size);
    
    memset(cc, 0, size);
    cc->tag = tag;

    uint16_t idx = g->consts.size;
    g->consts.tab[idx] = cc;
    g->consts.size++;

    *(er_const_t **)c = cc;

    return idx;
}

static uint16_t er_make_const_str(er_genctx_t *g, er_str_t *s) {
    er_const_str_t *str;
    uint16_t idx = er_add_const(g, &str, ER_CONST_STR, 
                                sizeof(er_const_str_t) + s->len);

    memcpy(&str->data, s->data, s->len);
    str->len = s->len;

    return idx;
}

static void er_codegen_node(er_genctx_t *g, er_irnode_t *node) {    
    switch (node->tag) {
        case ER_IR_PUSHINT: {
            er_emit(g, ER_OPC_LOAD_INT, node->data.s64);
            break;
        }

        case ER_IR_RET: {
            er_emit(g, ER_OPC_RETURN, 0);
            break;
        }

        case ER_IR_BINOP: {
            er_opcode_t opc;

            switch (node->data.binop) {
                case ER_BINOP_ADD:  
                    opc = ER_OPC_ADD_INT; 
                    break;

                case ER_BINOP_SUB:  
                    opc = ER_OPC_SUB_INT; 
                    break;

                case ER_BINOP_MUL:
                    opc = ER_OPC_MUL_INT;
                    break;
                    
                case ER_BINOP_DIV:
                    opc = ER_OPC_DIV_INT;
                    break;

                default:
                    ER_UNHANDLED_SWITCH_VALUE("%d", node->data.binop);
            }

            er_emit(g, opc, 0);
            break;
        }

        default:
            ER_UNHANDLED_SWITCH_VALUE("%d", node->tag);
    }
}

static void er_codegen_block(er_genctx_t *g, er_irblock_t *block) {
    for (size_t i = 0; i < block->n_nodes; i++) {
        er_codegen_node(g, &block->nodes[i]);
    }
}

static int er_assemble_instr(er_ginstr_t *instr, uint8_t *code, size_t at) {
    er_instrfmt_t fmt = er_opcode_format(instr->opc);

    code[at] = instr->opc;
    switch (fmt) {
        case ER_FMT_NONE:
            break;

        case ER_FMT_S16:
        case ER_FMT_INDEX:
        case ER_FMT_JUMP:
            er_write_u16(&code[at + 1], instr->data);
            break;

        default:
            ER_UNHANDLED_SWITCH_VALUE("%d", fmt);
    }

    return er_instr_size(instr->opc);
}

static void er_codegen_func(er_genctx_t *g, er_buildfunc_t *bfunc) {
    assert(g->func.code_size == 0); /* Func is cleared */

    assert(bfunc->n_blocks == 1);
    er_codegen_block(g, &bfunc->blocks[0]);

    er_genctx_print(g);
}

static uint16_t er_generate_me(er_genctx_t *g, er_buildmod_t *bmod) {
    er_const_modref_t *modref;
    uint16_t me = er_add_const(g, &modref, ER_CONST_MODREF, 
                               sizeof(er_const_modref_t));

    assert(me == 0);

    er_str_t name;
    er_str_from_cstr(&name, bmod->filename);
    modref->name = er_make_const_str(g, &name);

    return me;
}

static er_func_t *er_assemble_func(er_genctx_t *g, er_buildfunc_t *bfunc) {
    size_t code_size = 0;
    for (size_t i = 0; i < g->func.code_size; i++) {
        er_opcode_t opc = g->func.code[i].opc;

        if (opc != ER_CINSTR_LABEL) {
            code_size += er_instr_size(opc);
        }
    }

    uint8_t *code = er_xmalloc(code_size);

    size_t at = 0;
    for (size_t i = 0; i < g->func.code_size; i++) {
        er_opcode_t opc = g->func.code[i].opc;

        if (opc != ER_CINSTR_LABEL) {
            at += er_assemble_instr(&g->func.code[i], code, at);
        }
    }

    assert(code_size == at);

    er_constidx_t name = er_make_const_str(g, bfunc->name);
    return er_func_new(name, 0, code, code_size, 0, 0, bfunc->n_temps);
}

static er_mod_t *er_assemble_mod(er_genctx_t *g, er_constidx_t me, uint16_t id,
                                 er_func_t **funcs, size_t n_funcs) {
    er_const_t **consts = g->consts.tab;
    g->consts.tab = NULL;
    return er_mod_new(me, id, consts, g->consts.size, funcs, n_funcs);
}

void er_codegen_mod(er_buildmod_t *bmod) {
    er_genctx_t g;
    er_genctx_init(&g);

    uint16_t me = er_generate_me(&g, bmod);

    er_func_t **funcs = er_xmalloc(bmod->n_bfuncs * sizeof(er_func_t));
    for (size_t i = 0; i < bmod->n_bfuncs; i++) {
        er_buildfunc_t *bfunc = &bmod->bfuncs[i];

        er_genctx_clear_func(&g);

        er_codegen_func(&g, bfunc);
        funcs[i] = er_assemble_func(&g, bfunc);
    }

    bmod->mod = er_assemble_mod(&g, me, 0, funcs, bmod->n_bfuncs);

    er_genctx_destruct(&g);
}

void er_codegen(er_buildmod_t *bmod) {
    er_codegen_mod(bmod);
}
