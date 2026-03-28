#include "compiler/irgen.h"
#include "compiler/ir.h"
#include "util/error.h"
#include "util/alloc.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct {
    er_buildmod_t *bmod;
    er_buildfunc_t *bfunc;

    er_irnode_t *code;
    size_t code_size;
    size_t code_cap;

    er_irblock_t *blocks;
    size_t blocks_size;
    size_t blocks_cap;
} er_genctx_t;

static void er_genctx_init(er_genctx_t *g, er_buildmod_t *bmod, 
                           er_buildfunc_t *bfunc) {
    g->bmod = bmod;
    g->bfunc = bfunc;

    g->code_size = 0;
    g->code_cap = 16;
    g->code = er_xmalloc(g->code_cap * sizeof(er_irnode_t));

    g->blocks_size = 0;
    g->blocks_cap = 4;
    g->blocks = er_xmalloc(g->blocks_cap * sizeof(er_irblock_t));
}

static void er_genctx_destruct(er_genctx_t *g) {
    free(g->code);
    g->code = NULL;
    
    free(g->blocks);
    g->blocks = NULL;
}

static void er_genctx_print(er_genctx_t *g) {
    for (size_t i = 0; i < g->code_size; i++) {
        er_irnode_print(&g->code[i]);
    }
}

static er_irnode_t *er_emit(er_genctx_t *g, er_irtag_t tag,
                            er_irdatakind_t datakind, er_textpos_t pos) {
    assert(er_ir_kind(tag) == datakind);
    ER_UNUSED(datakind);
    
    if (g->code_size + 1 > g->code_cap) {
        g->code_cap *= 2;
        g->code = er_xrealloc(g->code, g->code_size * sizeof(er_irnode_t));
    }

    er_irnode_t *node = &g->code[g->code_size];
    g->code_size++;

    node->tag = tag;
    node->pos = pos;
    
    return node;
}

static void er_emit_NONE(er_genctx_t *g, er_irtag_t tag, er_textpos_t pos) {
    assert(er_ir_kind(tag) == ER_IRDATA_NONE);
    er_emit(g, tag, ER_IRDATA_NONE, pos);
}

static void er_emit_S64(er_genctx_t *g, er_irtag_t tag, er_textpos_t pos, 
                        int64_t s64) {
    er_irnode_t *node = er_emit(g, tag, ER_IRDATA_S64, pos);
    node->data.s64 = s64;
}

static void er_emit_BINOP(er_genctx_t *g, er_irtag_t tag, er_textpos_t pos, 
                          er_binop_t binop) {
    er_irnode_t *node = er_emit(g, tag, ER_IRDATA_BINOP, pos);
    node->data.binop = binop;
}

static void er_lower_expr(er_genctx_t *g, er_astnode_t *exprnode) {
    er_textpos_t pos = exprnode->pos;

    switch (exprnode->kind) {
        case ER_AST_BINOP: {
            er_astbinop_t *BinOp = &exprnode->data.BinOp;

            er_lower_expr(g, BinOp->left);
            er_lower_expr(g, BinOp->right);

            er_emit_BINOP(g, ER_IR_BINOP, pos, BinOp->op);
            break;
        }

        case ER_AST_INT: {
            er_astint_t *Int = &exprnode->data.Int;
            er_emit_S64(g, ER_IR_PUSHINT, pos, Int->val);
            break;
        }

        default:
            ER_UNHANDLED_SWITCH_VALUE("%d", exprnode->kind);
    }
}

static void er_lower_stmt(er_genctx_t *g, er_astnode_t *stmtnode) {
    er_textpos_t pos = stmtnode->pos;

    switch (stmtnode->kind) {
        case ER_AST_RET: {
            er_astret_t *Ret = &stmtnode->data.Ret;
            
            if (Ret->val != NULL) {
                er_lower_expr(g, Ret->val);
            } else {
                // TODO: Emit PUSH_NONE
            }

            er_emit_NONE(g, ER_IR_RET, pos);
            break;
        }

        default:
            ER_UNHANDLED_SWITCH_VALUE("%d", stmtnode->kind);
    }
}

static void er_lower_func(er_genctx_t *g, er_astnode_t *funcnode) {
    er_astfunc_t *func = &funcnode->data.Func;

    for (size_t i = 0; i < func->n_stmts; i++) {
        er_lower_stmt(g, func->stmts[i]);
    }
}

static er_irnode_t *er_move_code(er_genctx_t *g) {
    er_arena_t *arena = g->bfunc->arenas.ir;
    return er_arena_realloc(arena, g->code, 
                            g->code_size, g->code_size, 
                            sizeof(er_irnode_t));
}

static er_irblock_t *er_add_block(er_genctx_t *g, 
                                  er_irnode_t *nodes, size_t n_nodes) {
    if (g->blocks_size + 1 > g->blocks_cap) {
        g->blocks_cap *= 2;
        g->blocks = er_xrealloc(g->blocks,  
                                g->blocks_cap * sizeof(er_irblock_t));
    }

    er_irblock_t *block = &g->blocks[g->blocks_size];
    g->blocks_size++;

    block->nodes = nodes;
    block->n_nodes = n_nodes;

    return block;
}

static void er_move_blocks(er_genctx_t *g) {
    er_arena_t *arena = g->bfunc->arenas.ir;
    g->bfunc->blocks = er_arena_realloc(arena, g->blocks, 
                                        g->blocks_size, g->blocks_size, 
                                        sizeof(er_irblock_t));
    g->bfunc->n_blocks = g->blocks_size;
}

static void er_build_cfg(er_genctx_t *g) {
    er_irnode_t *code = er_move_code(g);
    
    er_add_block(g, code, g->code_size);
    
    er_move_blocks(g);
}

static void er_irgen_func(er_buildmod_t *bmod, er_buildfunc_t *bfunc) {
    er_genctx_t g;
    er_genctx_init(&g, bmod, bfunc);

    er_lower_func(&g, bfunc->root);
    er_build_cfg(&g);
    
    er_genctx_destruct(&g);

    for (size_t i = 0; i < bfunc->n_blocks; i++) {
        er_irblock_print(&bfunc->blocks[i]);
    }
}

void er_irgen(er_buildmod_t *bmod) {
    for (size_t i = 0; i < bmod->n_bfuncs; i++) {
        er_irgen_func(bmod, &bmod->bfuncs[i]);
    }
}
