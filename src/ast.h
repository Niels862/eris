#ifndef ERIS_AST_H
#define ERIS_AST_H

#include "token.h"
#include "ctk/rtti.h"
#include "ctk/list.h"
#include <stdint.h>
#include <stdbool.h>

#define ERIS_AST_NODES(X) \
        X(node) \
        X(node_stmt) \
        X(node_expr) \
        X(node_type) \
        X(node_decl) \
        X(node_source) \
        X(node_function_decl) \
        X(node_expr_stmt) \
        X(node_return) \
        X(node_intlit)

#define ERIS_DECLARE_AST_RTTI(t) CTK_RTTI_DECL(eris, t)
#define ERIS_DEFINE_AST_RTTI(t) CTK_RTTI_DEFN(eris, t)

typedef struct {
    ctk_rtti_t *meta;
} eris_node_t;

typedef struct {
    eris_node_t node;
} eris_node_stmt_t;

typedef struct {
    eris_node_t node;
} eris_node_expr_t;

typedef struct {
    eris_node_t node;
} eris_node_type_t;

typedef struct {
    eris_node_stmt_t stmt;
    ctk_token_t *name;
} eris_node_decl_t;

typedef struct {
    eris_node_t node;
    eris_node_stmt_t **stmts;
} eris_node_source_t;

typedef struct {
    eris_node_decl_t decl;
    eris_node_stmt_t **stmts;
} eris_node_function_decl_t;

typedef struct {
    eris_node_stmt_t stmt;
    eris_node_expr_t *expr;
} eris_node_expr_stmt_t;

typedef struct {
    eris_node_stmt_t stmt;
    ctk_token_t *token;
    eris_node_expr_t *value;
} eris_node_return_t;

typedef struct {
    eris_node_expr_t expr;
    ctk_token_t *token;
    int64_t value;
} eris_node_intlit_t;

eris_node_source_t *eris_node_source_new(ctk_list_t *decls);

eris_node_decl_t *eris_node_function_decl_new(ctk_token_t *name, 
                                              eris_node_stmt_t **stmts);

eris_node_stmt_t *eris_node_expr_stmt_new(eris_node_expr_t *expr);

eris_node_stmt_t *eris_node_return_new(ctk_token_t *token, 
                                       eris_node_expr_t *value);

eris_node_expr_t *eris_node_intlit_new(ctk_token_t *token, int64_t value);

ERIS_AST_NODES(ERIS_DECLARE_AST_RTTI)

#endif
