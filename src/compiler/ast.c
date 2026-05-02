#include "compiler/ast.h"
#include "util/error.h"
#include <inttypes.h>
#include <assert.h>

#define X(n) [ER_AST_##n] = #n,
static const char * const er_ast_names[] = {
    ER_AST_NODES(X)
};
#undef X

static char const *er_ast_name(er_astkind_t kind) {
    if (kind < sizeof(er_ast_names) / sizeof(*er_ast_names)) {
        return er_ast_names[kind];
    }
    return NULL;
}

#define X(n) [ER_BINOP_##n] = #n,
static const char * const er_binop_names[] = {
    ER_BINOPS(X)
};
#undef X

char const *er_binop_name(er_binop_t op) {
    if (op < sizeof(er_binop_names) / sizeof(*er_binop_names)) {
        return er_binop_names[op];
    }
    return NULL;
}

static void er_print_indent(size_t depth) {
    for (size_t i = 0; i < depth; i++) {
        fprintf(stderr, "  ");
    }
}

static void er_print_header(char const *attr, size_t depth) {
    er_print_indent(depth);
    if (*attr != '\0') {
        fprintf(stderr, "%s: ", attr);
    }
}

static void er_print_node(char const *attr, 
                          er_astnode_t *n, size_t depth);

static void er_print_list(char const *attr, 
                          er_astnode_t **ns, size_t size, size_t depth) {    
    er_print_header(attr, depth);
    
    if (size == 0) {
        fprintf(stderr, "[]\n");
        return;
    }

    assert(ns != NULL);

    fprintf(stderr, "[\n");

    for (size_t i = 0; i < size; i++) {
        er_print_node("", ns[i], depth + 1);
    }

    er_print_indent(depth);
    fprintf(stderr, "]\n");
}

static void er_print_str(char const *attr, er_str_t *str, size_t depth) {
    er_print_header(attr, depth);
    fprintf(stderr, "%.*s\n", str->len, str->data);
}

static void er_print_node(char const *attr, 
                          er_astnode_t *n, size_t depth) {    
    er_print_header(attr, depth);
                                    
    if (n == NULL) {
        fprintf(stderr, "(null)");
        return;
    }

    fprintf(stderr, "%s at %d:%d {\n", 
            er_ast_name(n->kind), n->pos.line, n->pos.col);

    er_astdata_t *data = &n->data;
    size_t ndepth = depth + 1;

    switch (n->kind) {
        case ER_AST_NONE:
            return;

        case ER_AST_MOD: {
            er_astmod_t *Mod = &data->Mod;
            er_print_list("funcs", Mod->funcs, Mod->n_funcs, ndepth);
            break;
        }

        case ER_AST_FUNC: {
            er_astfunc_t *Func = &data->Func;
            er_print_str("name", &Func->name, ndepth);
            er_print_node("ret_anno", Func->ret_anno, ndepth);
            er_print_list("stmts", Func->stmts, Func->n_stmts, ndepth);
            break;
        }

        case ER_AST_VAR: {
            er_astvar_t *Var = &data->Var;
            er_print_str("name", &Var->name, ndepth);
            er_print_node("anno", Var->anno, ndepth);
            er_print_node("value", Var->value, ndepth);
            break;
        }

        case ER_AST_RET: {
            er_astret_t *Ret = &data->Ret;
            er_print_node("value", Ret->val, ndepth);
            break;
        }

        case ER_AST_INT: {
            er_astint_t *Int = &data->Int;
            er_print_header("value", ndepth);
            fprintf(stderr, "%" PRId64 "\n", Int->val);
            break;
        }

        case ER_AST_BINOP: {
            er_astbinop_t *BinOp = &data->BinOp;
            er_print_header("op", ndepth);
            fprintf(stderr, "%s\n", er_binop_name(BinOp->op));
            er_print_node("left", BinOp->left, ndepth);
            er_print_node("right", BinOp->right, ndepth);
            break;
        }

        case ER_AST_IDENT: {
            er_astident_t *Ident = &data->Ident;
            er_print_str("name", &Ident->name, ndepth);
            break;
        }
    }

    er_print_indent(depth);
    fprintf(stderr, "}\n");
}

void er_ast_print(er_astnode_t *n) {
    er_print_node("root", n, 0);
}