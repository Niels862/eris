#include "code-generator.h"
#include "ast.h"
#include "instruction.h"
#include "constant-table.h"
#include "ctk/rtti.h"
#include "ctk/list.h"
#include <assert.h>

typedef struct eris_codegen_t eris_codegen_t;

typedef void(*eris_codegen_visit_t)(eris_codegen_t *gen, void *node);

struct eris_codegen_t {
    ctk_dynarr_t code;
    ctk_list_t ctable;
    ctk_pool_t cpool;
    eris_codegen_visit_t *visitors;
};

static void eris_codegen_init(eris_codegen_t *gen, eris_codegen_visit_t *visitors) {
    ctk_dynarr_init(&gen->code, 16);
    ctk_list_init(&gen->ctable, 16);
    ctk_pool_init(&gen->cpool);
    gen->visitors = visitors;
}

static void eris_codegen_move_to_module(eris_codegen_t *gen, 
                                        eris_module_t *mod) {
    eris_module_init(mod, 
        ctk_dynarr_move(&gen->code), gen->code.size,
        ctk_list_move(&gen->ctable), ctk_list_size(&gen->ctable), 
        &gen->cpool);
}

static void *eris_codegen_add_const(eris_codegen_t *gen, 
                                    eris_centry_kind_t kind, 
                                    eris_cindex_t *idx) {
    size_t size = eris_centry_size[kind];
    size_t align = eris_centry_align[kind];

    *idx = ctk_list_size(&gen->ctable);

    void *c = ctk_pool_alloc_aligned(&gen->cpool, size, align);
    ctk_list_add(&gen->ctable, c);

    *(uint8_t *)c = kind;

    assert((uintptr_t)c % align == 0);

    return c;
}

static void eris_emit_instr(eris_codegen_t *gen, eris_instr_t instr) {
    uint8_t *p = ctk_dynarr_add(&gen->code, 1);
    p[0] = instr;
}

static void eris_emit_instr_with_cindex(eris_codegen_t *gen, 
                                        eris_instr_t instr, 
                                        eris_cindex_t index) {
    uint8_t *p = ctk_dynarr_add(&gen->code, 3);
    p[0] = instr;
    p[1] = index & 0xFF;
    p[2] = (index >> 8) & 0xFF;
}

static void eris_emit_instr_with_s16(eris_codegen_t *gen, eris_instr_t instr, 
                                     int16_t s16) {
    uint16_t u16 = s16;
    
    uint8_t *p = ctk_dynarr_add(&gen->code, 3);
    p[0] = instr;
    p[1] = u16 & 0xFF;
    p[2] = (u16 >> 8) & 0xFF;            
}

#define ERIS_EMITTER_NONE(e, s) \
        inline static void eris_emit_##s(eris_codegen_t *gen) \
            { eris_emit_instr(gen, ERIS_INSTR_##e); }

#define ERIS_EMITTER_CINDEX(e, s) \
        inline static void eris_emit_##s(eris_codegen_t *gen, \
                                         eris_clabel_t label) \
            { eris_emit_instr_with_cindex(gen, ERIS_INSTR_##e, label); }

#define ERIS_EMITTER_S16(e, s) \
        inline static void eris_emit_##s(eris_codegen_t *gen, \
                                         eris_clabel_t label) \
            { eris_emit_instr_with_s16(gen, ERIS_INSTR_##e, label); }

#define ERIS_INSTR_X_EXPAND_EMIT(e, s, f) ERIS_EMITTER_##f(e, s)

ERIS_INSTRS(ERIS_INSTR_X_EXPAND_EMIT)

static void eris_codegen_dispatch(eris_codegen_t *gen, void *node) {
    ctk_rtti_t *meta = CTK_RTTI_META(node);
    assert(meta->id != 0);
    assert(eris_node_instanceof(node));

    eris_codegen_visit_t visit = gen->visitors[meta->id];
    assert(visit != NULL);

    visit(gen, node);
}

static void eris_codegen_source(eris_codegen_t *gen, void *vnode) {
    eris_node_source_t *node = eris_node_source_dyncast(vnode);

    for (size_t i = 0; node->stmts[i] != NULL; i++) {
        eris_codegen_dispatch(gen, node->stmts[i]);
    }
}

static void eris_codegen_function_decl(eris_codegen_t *gen, void *vnode) {
    eris_node_function_decl_t *node = eris_node_function_decl_dyncast(vnode);

    eris_cindex_t idx;
    eris_const_function_t *c 
            = eris_codegen_add_const(gen, ERIS_CONST_FUNCTION, &idx);

    c->codestart = gen->code.size;

    for (size_t i = 0; node->stmts[i] != NULL; i++) {
        eris_codegen_dispatch(gen, node->stmts[i]);
    }

    c->codesize = gen->code.size - c->codestart;

    (void)c;
}

static void eris_codegen_expr_stmt(eris_codegen_t *gen, void *vnode) {
    eris_node_expr_stmt_t *node = eris_node_expr_stmt_dyncast(vnode);

    eris_codegen_dispatch(gen, node->expr);
    eris_emit_pop(gen);
}

static void eris_codegen_return(eris_codegen_t *gen, void *vnode) {
    eris_node_return_t *node = eris_node_return_dyncast(vnode);

    eris_codegen_dispatch(gen, node->value);
    eris_emit_ireturn(gen);
}

static void eris_codegen_intlit(eris_codegen_t *gen, void *vnode) {
    eris_node_intlit_t *node = eris_node_intlit_dyncast(vnode);

    eris_emit_ipush(gen, node->value);
}

void eris_codegen(eris_node_source_t *root, eris_module_t *mod) {
    static eris_codegen_visit_t visitors[] = {
        [CTK_NODE_SOURCE]           = &eris_codegen_source,
        [CTK_NODE_FUNCTION_DECL]    = &eris_codegen_function_decl,
        [CTK_NODE_EXPR_STMT]        = &eris_codegen_expr_stmt,
        [CTK_NODE_RETURN]           = &eris_codegen_return,
        [CTK_NODE_INTLIT]           = &eris_codegen_intlit,
    };
    
    eris_codegen_t gen;
    eris_codegen_init(&gen, visitors);

    eris_codegen_dispatch(&gen, root);

    eris_codegen_move_to_module(&gen, mod);
}
