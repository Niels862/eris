#include "compiler/lower.h"
#include "compiler/ir.h"
#include "util/error.h"
#include "util/alloc.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct {
    er_buildmod_t *bmod;

    er_irnode_t *code;
    size_t code_size;
    size_t code_cap;
} er_genctx_t;

static void er_genctx_init(er_genctx_t *g, er_buildmod_t *bmod) {
    g->bmod = bmod;
    g->code_size = 0;
    g->code_cap = 16;
    g->code = er_xmalloc(g->code_cap * sizeof(er_irnode_t));
}

static void er_genctx_destruct(er_genctx_t *g) {
    free(g->code);
    g->code = NULL;
}

static void er_genctx_print(er_genctx_t *g) {
    for (size_t i = 0; i < g->code_size; i++) {
        er_irnode_print(&g->code[i]);
    }
}

static er_irnode_t *er_emit(er_genctx_t *g, er_irtag_t tag, 
                            er_textpos_t pos) {
    if (g->code_size + 1 > g->code_cap) {
        g->code_cap *= 2;
        g->code = er_xrealloc(g->code, g->code_size * sizeof(er_irnode_t));
    }

    er_irnode_t *node = &g->code[g->code_size];
    g->code_size += 1;

    node->tag = tag;
    node->pos = pos;
    

    return node;
}

static void er_emit_NONE(er_genctx_t *g, er_irtag_t tag, er_textpos_t pos) {
    assert(er_ir_kind(tag) == ER_IRDATA_NONE);
    er_emit(g, tag, pos);
}

static void er_emit_S64(er_genctx_t *g, er_irtag_t tag, er_textpos_t pos, 
                        int64_t s64) {
    assert(er_ir_kind(tag) == ER_IRDATA_S64);
    er_irnode_t *node = er_emit(g, tag, pos);
    node->tag = tag;
    node->data.s64 = s64;
}

static void er_lower_expr(er_genctx_t *g, er_astnode_t *exprnode) {
    er_textpos_t pos = exprnode->pos;

    switch (exprnode->kind) {
        case ER_AST_INT: {
            er_astint_t *Int = &exprnode->data.Int;
            er_emit_S64(g, ER_IR_PUSHINT, pos, Int->val);
            break;
        }

        default:
            ER_FATAL("Unhandled switch value: %d", exprnode->kind);
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
            ER_FATAL("Unhandled switch value: %d", stmtnode->kind);
    }
}

static void er_lower_func(er_genctx_t *g, er_astnode_t *funcnode) {
    er_astfunc_t *func = &funcnode->data.Func;

    for (size_t i = 0; i < func->n_stmts; i++) {
        er_lower_stmt(g, func->stmts[i]);
    }
}

void er_irgen(er_buildmod_t *bmod, er_buildfunc_t *bfunc) {
    er_genctx_t g;
    er_genctx_init(&g, bmod);

    er_lower_func(&g, bfunc->root);

    er_genctx_print(&g);
    
    er_genctx_destruct(&g);
}
