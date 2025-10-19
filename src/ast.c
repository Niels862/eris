#include "ast.h"
#include <assert.h>

static ctk_rtti_t eris_node_rtti = {
    .name = "node-base",
    .super = &ctk_rtti_base,
};

static ctk_rtti_t eris_node_stmt_rtti = {
    .name = "node-stmt",
    .super = &eris_node_rtti,
};

static ctk_rtti_t eris_node_expr_rtti = {
    .name = "node-expr",
    .super = &eris_node_rtti,
};

static ctk_rtti_t eris_node_type_rtti = {
    .name = "node-type",
    .super = &eris_node_rtti,
};

static ctk_rtti_t eris_node_decl_rtti = {
    .name = "node-decl",
    .super = &eris_node_stmt_rtti,
    .attrs = CTK_RTTI_ATTR_LIST(
        CTK_RTTI_ATTR(eris_node_decl_t, name, CTK_TYPE_TOKEN)
    )
};

static ctk_rtti_t eris_node_source_rtti = {
    .name = "node-source",
    .super = &eris_node_rtti,
    .attrs = CTK_RTTI_ATTR_LIST(
        CTK_RTTI_ATTR(eris_node_source_t, stmts, CTK_TYPE_RTTI_LIST)
    )
};

static ctk_rtti_t eris_node_function_decl_rtti = {
    .name = "node-function-decl",
    .super = &eris_node_decl_rtti,
    .attrs = CTK_RTTI_ATTR_LIST(
        CTK_RTTI_ATTR(eris_node_function_decl_t, stmts, CTK_TYPE_RTTI_LIST)
    )
};

static ctk_rtti_t eris_node_expr_stmt_rtti = {
    .name = "node-expr-stmt",
    .super = &eris_node_stmt_rtti,
    .attrs = CTK_RTTI_ATTR_LIST(
        CTK_RTTI_ATTR(eris_node_expr_stmt_t, expr, CTK_TYPE_RTTI)
    )
};

static ctk_rtti_t eris_node_return_rtti = {
    .name = "node-return",
    .super = &eris_node_stmt_rtti,
    .attrs = CTK_RTTI_ATTR_LIST(
        CTK_RTTI_ATTR(eris_node_return_t, value, CTK_TYPE_RTTI)
    )
};

static ctk_rtti_t eris_node_intlit_rtti = {
    .name = "node-intlit",
    .super = &eris_node_expr_rtti,
    .attrs = CTK_RTTI_ATTR_LIST(
        CTK_RTTI_ATTR(eris_node_intlit_t, value, CTK_TYPE_I64),
        CTK_RTTI_ATTR(eris_node_intlit_t, token, CTK_TYPE_TOKEN)
    )
};

static void eris_node_init(eris_node_t *node, ctk_rtti_t *meta) {
    node->meta = meta;
    assert(eris_node_instanceof(node));
}

static void eris_node_stmt_init(eris_node_stmt_t *node, ctk_rtti_t *meta) {
    eris_node_init(&node->node, meta);
    assert(eris_node_stmt_instanceof(node));
}

static void eris_node_expr_init(eris_node_expr_t *node, ctk_rtti_t *meta) {
    eris_node_init(&node->node, meta);
    assert(eris_node_expr_instanceof(node));
}

/*static*/ void eris_node_type_init(eris_node_type_t *node, ctk_rtti_t *meta) {
    eris_node_init(&node->node, meta);
    assert(eris_node_type_instanceof(node));
}

static void eris_node_decl_init(eris_node_decl_t *node, ctk_token_t *name, 
                                ctk_rtti_t *meta) {
    eris_node_stmt_init(&node->stmt, meta);
    assert(eris_node_decl_instanceof(node));
    node->name = name;
}

eris_node_source_t *eris_node_source_new(ctk_list_t *decls) {
    eris_node_source_t *node = eris_node_source_xalloc();

    eris_node_init(&node->node, &eris_node_source_rtti);
    node->stmts = (eris_node_stmt_t **)ctk_list_move_raw(decls);

    return node;
}

eris_node_decl_t *eris_node_function_decl_new(ctk_token_t *name, 
                                              eris_node_stmt_t **stmts) {
    eris_node_function_decl_t *node = eris_node_function_decl_xalloc();

    eris_node_decl_init(&node->decl, name, &eris_node_function_decl_rtti);
    node->stmts = stmts;

    return &node->decl;
}

eris_node_stmt_t *eris_node_expr_stmt_new(eris_node_expr_t *expr) {
    eris_node_expr_stmt_t *node = eris_node_expr_stmt_xalloc();

    eris_node_stmt_init(&node->stmt, &eris_node_expr_stmt_rtti);
    node->expr = expr;

    return &node->stmt;
}

eris_node_stmt_t *eris_node_return_new(ctk_token_t *token, 
                                       eris_node_expr_t *value) {
    eris_node_return_t *node = eris_node_return_xalloc();

    eris_node_stmt_init(&node->stmt, &eris_node_return_rtti);
    node->token = token;
    node->value = value;

    return &node->stmt;
}

eris_node_expr_t *eris_node_intlit_new(ctk_token_t *token, int64_t value) {
    eris_node_intlit_t *node = eris_node_intlit_xalloc();

    eris_node_expr_init(&node->expr, &eris_node_intlit_rtti);
    node->token = token;
    node->value = value;

    return &node->expr;
}

ERIS_AST_NODES(ERIS_DEFINE_AST_RTTI)
