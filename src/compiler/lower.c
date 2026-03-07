#include "compiler/lower.h"
#include "compiler/ir.h"
#include "util/error.h"
#include "util/alloc.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
    er_buildmod_t *bmod;

    er_irnode_t *code;
    size_t code_size;
    size_t code_cap;
} er_lowerctx_t;

static void er_lowerctx_init(er_lowerctx_t *l, er_buildmod_t *bmod) {
    l->bmod = bmod;
    l->code_size = 0;
    l->code_cap = 16;
    l->code = er_xmalloc(l->code_cap * sizeof(er_irnode_t));
}

static void er_lowerctx_destruct(er_lowerctx_t *l) {
    free(l->code);
    l->code = NULL;
}

static void er_lowerctx_print(er_lowerctx_t *l) {
    for (size_t i = 0; i < l->code_size; i++) {
        er_irnode_print(&l->code[i]);
    }
}

static er_irnode_t *er_emit(er_lowerctx_t *l, er_irtag_t tag, 
                            er_textpos_t pos) {
    if (l->code_size + 1 > l->code_cap) {
        l->code_cap *= 2;
        l->code = er_xrealloc(l->code, l->code_size * sizeof(er_irnode_t));
    }

    er_irnode_t *node = &l->code[l->code_size];
    l->code_size += 1;

    node->tag = tag;
    node->pos = pos;
    

    return node;
}

static void er_emit_NONE(er_lowerctx_t *l, er_irtag_t tag, er_textpos_t pos) {
    er_emit(l, tag, pos);
}

static void er_emit_I64(er_lowerctx_t *l, er_irtag_t tag, er_textpos_t pos, 
                        int64_t s64) {
    er_irnode_t *node = er_emit(l, tag, pos);
    node->tag = tag;
    node->data.s64 = s64;
}

static void er_lower_expr(er_lowerctx_t *l, er_astnode_t *exprnode) {
    er_textpos_t pos = exprnode->pos;

    switch (exprnode->kind) {
        case ER_AST_INT: {
            er_astint_t *Int = &exprnode->data.Int;
            er_emit_I64(l, ER_IR_PUSHINT, pos, Int->val);
            break;
        }

        default:
            ER_FATAL("Unhandled switch value: %d", exprnode->kind);
    }
}

static void er_lower_stmt(er_lowerctx_t *l, er_astnode_t *stmtnode) {
    er_textpos_t pos = stmtnode->pos;

    switch (stmtnode->kind) {
        case ER_AST_RET: {
            er_astret_t *Ret = &stmtnode->data.Ret;
            
            if (Ret->val != NULL) {
                er_lower_expr(l, Ret->val);
            } else {
                // TODO: Emit PUSH_NONE
            }

            er_emit_NONE(l, ER_IR_RET, pos);
            break;
        }

        default:
            ER_FATAL("Unhandled switch value: %d", stmtnode->kind);
    }
}

static void er_lower_func(er_lowerctx_t *l, er_astnode_t *funcnode) {
    er_astfunc_t *func = &funcnode->data.Func;

    for (size_t i = 0; i < func->n_stmts; i++) {
        er_lower_stmt(l, func->stmts[i]);
    }
}

void er_lower(er_buildmod_t *bmod, er_astnode_t *funcnode) {
    er_lowerctx_t l;
    er_lowerctx_init(&l, bmod);

    er_lower_func(&l, funcnode);

    er_lowerctx_print(&l);
    
    er_lowerctx_destruct(&l);
}
