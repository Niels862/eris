#include "backend/code-generator.h"
#include "backend/code-generator-util.h"
#include "frontend/ast.h"
#include "runtime/instruction.h"
#include "runtime/constant-table.h"
#include "ctk/rtti.h"
#include "ctk/list.h"
#include <assert.h>

static void eris_codegen_source(eris_codegen_t *gen, void *vnode) {
    eris_node_source_t *node = eris_node_source_dyncast(vnode);

    for (size_t i = 0; node->stmts[i] != NULL; i++) {
        eris_codegen_dispatch(gen, node->stmts[i]);
    }
}

static void eris_codegen_function_decl(eris_codegen_t *gen, void *vnode) {
    eris_node_function_decl_t *node = eris_node_function_decl_dyncast(vnode);

    eris_cindex_t fidx;
    eris_const_function_t *c = eris_codegen_add_const(gen, ERIS_CONST_FUNCTION, 
                                                      0, &fidx);

    ctk_strspan_t *name = &node->decl.name->lexeme;
    c->name = eris_codegen_add_str(gen, name->start, name->end - name->start);

    c->codestart = gen->code.size;

    for (size_t i = 0; node->stmts[i] != NULL; i++) {
        eris_codegen_dispatch(gen, node->stmts[i]);
    }

    c->codesize = gen->code.size - c->codestart;
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
