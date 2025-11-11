#include "frontend/file-parser.h"
#include "frontend/parser.h"
#include <assert.h>

static eris_node_expr_t *eris_parse_expr(eris_parser_t *p);

static eris_node_type_t *eris_parse_type(eris_parser_t *p) {
    ctk_token_t *curr = eris_parser_curr(p);

    ctk_token_t *name = eris_parser_accept(p, ERIS_TOKEN_IDENTIFIER);
    if (name != NULL) {
        return eris_node_named_type_new(name);
    }

    eris_parser_expect_error(p, curr, 0, "expected type");
    return NULL;
}

// FIXME: overflow, errors, ...
static int64_t eris_parse_number(ctk_token_t *num) {
    int64_t s64 = 0;

    for (char *s = num->lexeme.start; s != num->lexeme.end; s++) {
        s64 = s64 * 10 + (*s - '0');
    }

    return s64;
}

static eris_node_expr_t *eris_parse_atom(eris_parser_t *p) {
    eris_node_expr_t *expr = NULL;
    ctk_token_t *curr = eris_parser_curr(p);

    switch (curr->kind) {
        case ERIS_TOKEN_NUMBER: {
            eris_parser_advance(p);
            expr = eris_node_intlit_new(curr, eris_parse_number(curr));
            break;
        }

        case ERIS_TOKEN_LPAREN: {
            eris_parser_advance(p);
            expr = eris_parse_expr(p);
            if (!eris_parser_expect(p, ERIS_TOKEN_RPAREN, NULL)) {
                goto error;
            }
            break;
        }

        default: {
            eris_parser_expect_error(p, curr, 0, "expected value");
            goto error;
        }
    }

    return expr;

error:
    return NULL;
}

static eris_node_expr_t *eris_parse_expr(eris_parser_t *p) {
    return eris_parse_atom(p);
}

static eris_node_stmt_t *eris_parse_return(eris_parser_t *p) {
    eris_node_expr_t *value = NULL;
    
    ctk_token_t *token = eris_parser_expect(p, ERIS_TOKEN_RETURN, NULL);
    if (token == NULL) {
        goto error;
    }

    value = eris_parse_expr(p);
    if (value == NULL) {
        goto error;
    }

    if (!eris_parser_expect(p, ERIS_TOKEN_SEMICOLON, NULL)) {
        goto error;
    }

    return eris_node_return_new(token, value);

error:
    ctk_rtti_delete(value);
    return NULL;
}

static eris_node_stmt_t *eris_parse_expr_stmt(eris_parser_t *p) {
    eris_node_expr_t *expr = eris_parse_expr(p);
    if (expr == NULL) {
        goto error;
    }

    if (!eris_parser_expect(p, ERIS_TOKEN_SEMICOLON, NULL)) {
        goto error;
    }
    
    return eris_node_expr_stmt_new(expr);

error:
    ctk_rtti_delete(expr);
    return NULL;
}

static eris_node_stmt_t *eris_parse_stmt(eris_parser_t *p) {
    switch (eris_parser_curr(p)->kind) {
        case ERIS_TOKEN_RETURN:     
            return eris_parse_return(p);

        default:                    
            return eris_parse_expr_stmt(p);
    }
}

static eris_node_stmt_t **eris_parse_stmt_body(eris_parser_t *p) {
    ctk_list_t stmts;
    ctk_list_init(&stmts, 4);

    eris_node_stmt_t *stmt = NULL;

    if (!eris_parser_expect(p, ERIS_TOKEN_LBRACE, NULL)) {
        goto error;
    }

    bool has_error = false;
    while (eris_parser_curr(p)->kind != ERIS_TOKEN_RBRACE) {
        eris_node_stmt_t *stmt = eris_parse_stmt(p);
        if (stmt == NULL) {
            has_error = true;

            if (eris_parser_curr(p)->kind != ERIS_TOKEN_RBRACE) {
                eris_parser_advance(p);
            }
        } else {
            ctk_list_add(&stmts, stmt);
            stmt = NULL;
        }

        if (eris_parser_at_end(p)) {
            // Temporary bypass. parser_get_curr should be configurable to 
            // either keep repeating the final token or to return NONE
            ctk_token_t *curr = eris_parser_curr(p);
            eris_parser_expect_error(p, curr, ERIS_TOKEN_RBRACE, NULL);
            
            goto error;
        }
    }

    if (has_error) {
        goto error;
    } else {
        eris_parser_expect(p, ERIS_TOKEN_RBRACE, "expected '}");
    }

    return (eris_node_stmt_t **)ctk_list_move(&stmts);

error:
    ctk_rtti_delete(stmt);
    ctk_list_destruct(&stmts);
    return NULL;
}

static eris_node_decl_t *eris_parse_function_decl(eris_parser_t *p) {
    eris_node_type_t *rettype = NULL;
    eris_node_stmt_t **stmts = NULL;
    
    if (!eris_parser_expect(p, ERIS_TOKEN_FUNCTION, NULL)) {
        goto error;
    }

    ctk_token_t *name = eris_parser_expect(p, ERIS_TOKEN_IDENTIFIER, NULL);
    if (name == NULL) {
        goto error;
    }

    if (!eris_parser_expect(p, ERIS_TOKEN_LPAREN, NULL)) {
        goto error;
    }
    
    if (!eris_parser_expect(p, ERIS_TOKEN_RPAREN, NULL)) {
        goto error;
    }

    if (eris_parser_accept(p, ERIS_TOKEN_ARROW)) {
        rettype = eris_parse_type(p);
        if (rettype == NULL) {
            goto error;
        }
    }

    stmts = eris_parse_stmt_body(p);
    if (stmts == NULL) {
        goto error;
    }

    return eris_node_function_decl_new(name, rettype, stmts);

error:
    ctk_rtti_delete(rettype);
    ctk_rtti_list_delete((void **)stmts);
    return NULL;
}

static eris_node_decl_t *eris_parse_decl(eris_parser_t *p) {
    ctk_token_t *curr = eris_parser_curr(p);

    switch (curr->kind) {
        case ERIS_TOKEN_FUNCTION:
            return eris_parse_function_decl(p);
        
        default:
            eris_parser_expect_error(p, curr, 0, "expected declaration");
            return NULL;
    } 
}

eris_node_source_t *eris_parse_file(ctk_span_t *span, 
                                    eris_scopelist_t *scopes) {
    (void)scopes;
    
    eris_parser_t p;
    eris_parser_init(&p, span);

    ctk_list_t decls;
    ctk_list_init(&decls, 4);

    bool has_error = false;
    while (!eris_parser_at_end(&p)) {
        eris_node_decl_t *decl = eris_parse_decl(&p);

        if (decl == NULL) {
            has_error = true;
            eris_parser_advance(&p);
        } else {
            ctk_list_add(&decls, decl);
        }
    }
    
    if (has_error) {
        goto error;
    }

    return eris_node_source_new(&decls);

error:
    ctk_list_destruct(&decls);
    return NULL;
}
