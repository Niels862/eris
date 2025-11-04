#include "file-parser.h"
#include "parser.h"
#include <assert.h>

static eris_node_expr_t *eris_parse_expr(eris_parser_t *parser);

static eris_node_type_t *eris_parse_type(ctk_parser_t *parser) {
    ctk_token_t *curr = parser->curr;

    ctk_token_t *name = ctk_parser_accept(parser, ERIS_TOKEN_IDENTIFIER);
    if (name != NULL) {
        return eris_node_named_type_new(name);
    }

    parser->expect_error(curr, 0, "expected type");
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

static eris_node_expr_t *eris_parse_atom(eris_parser_t *parser) {
    eris_node_expr_t *expr = NULL;
    ctk_token_t *curr = parser->curr;

    switch (curr->kind) {
        case ERIS_TOKEN_NUMBER: {
            ctk_parser_advance(parser);
            expr = eris_node_intlit_new(curr, eris_parse_number(curr));
            break;
        }

        case ERIS_TOKEN_LPAREN: {
            ctk_parser_advance(parser);
            expr = eris_parse_expr(parser);
            if (!ctk_parser_expect(parser, ERIS_TOKEN_RPAREN, NULL)) {
                goto error;
            }
            break;
        }

        default: {
            parser->expect_error(curr, 0, "expected value");
            goto error;
        }
    }

    return expr;

error:
    return NULL;
}

static eris_node_expr_t *eris_parse_expr(eris_parser_t *parser) {
    return eris_parse_atom(parser);
}

static eris_node_stmt_t *eris_parse_return(eris_parser_t *parser) {
    eris_node_expr_t *value = NULL;
    
    ctk_token_t *token = ctk_parser_expect(parser, ERIS_TOKEN_RETURN, NULL);
    if (token == NULL) {
        goto error;
    }

    value = eris_parse_expr(parser);
    if (value == NULL) {
        goto error;
    }

    if (!ctk_parser_expect(parser, ERIS_TOKEN_SEMICOLON, NULL)) {
        goto error;
    }

    return eris_node_return_new(token, value);

error:
    ctk_rtti_delete(value);
    return NULL;
}

static eris_node_stmt_t *eris_parse_expr_stmt(eris_parser_t *parser) {
    eris_node_expr_t *expr = eris_parse_expr(parser);
    if (expr == NULL) {
        goto error;
    }

    if (!ctk_parser_expect(parser, ERIS_TOKEN_SEMICOLON, NULL)) {
        goto error;
    }
    
    return eris_node_expr_stmt_new(expr);

error:
    ctk_rtti_delete(expr);
    return NULL;
}

static eris_node_stmt_t *eris_parse_stmt(eris_parser_t *parser) {
    switch (ctk_parser_curr(parser)->kind) {
        case ERIS_TOKEN_RETURN:     
            return eris_parse_return(parser);

        default:                    
            return eris_parse_expr_stmt(parser);
    }
}

static eris_node_stmt_t **eris_parse_stmt_body(eris_parser_t *parser) {
    ctk_list_t stmts;
    ctk_list_init(&stmts, 4);

    eris_node_stmt_t *stmt = NULL;

    if (!ctk_parser_expect(parser, ERIS_TOKEN_LBRACE, NULL)) {
        goto error;
    }

    bool has_error = false;
    while (ctk_parser_curr(parser)->kind != ERIS_TOKEN_RBRACE) {
        eris_node_stmt_t *stmt = eris_parse_stmt(parser);
        if (stmt == NULL) {
            has_error = true;

            if (ctk_parser_curr(parser)->kind != ERIS_TOKEN_RBRACE) {
                ctk_parser_advance(parser);
            }
        } else {
            ctk_list_add(&stmts, stmt);
            stmt = NULL;
        }

        if (ctk_parser_at_end(parser)) {
            // Temporary bypass. parser_get_curr should be configurable to 
            // either keep repeating the final token or to return NONE
            ctk_token_t *curr = parser->curr;
            parser->expect_error(curr, ERIS_TOKEN_RBRACE, NULL);
            
            goto error;
        }
    }

    if (has_error) {
        goto error;
    } else {
        ctk_parser_expect(parser, ERIS_TOKEN_RBRACE, "expected '}");
    }

    return (eris_node_stmt_t **)ctk_list_move(&stmts);

error:
    ctk_rtti_delete(stmt);
    ctk_list_destruct(&stmts);
    return NULL;
}

static eris_node_decl_t *eris_parse_function_decl(eris_parser_t *parser) {
    eris_node_type_t *rettype = NULL;
    eris_node_stmt_t **stmts = NULL;
    
    if (!ctk_parser_expect(parser, ERIS_TOKEN_FUNCTION, NULL)) {
        goto error;
    }

    ctk_token_t *name = ctk_parser_expect(parser, ERIS_TOKEN_IDENTIFIER, NULL);
    if (name == NULL) {
        goto error;
    }

    if (!ctk_parser_expect(parser, ERIS_TOKEN_LPAREN, NULL)) {
        goto error;
    }
    
    if (!ctk_parser_expect(parser, ERIS_TOKEN_RPAREN, NULL)) {
        goto error;
    }

    if (ctk_parser_accept(parser, ERIS_TOKEN_ARROW)) {
        rettype = eris_parse_type(parser);
        if (rettype == NULL) {
            goto error;
        }
    }

    stmts = eris_parse_stmt_body(parser);
    if (stmts == NULL) {
        goto error;
    }

    return eris_node_function_decl_new(name, rettype, stmts);

error:
    ctk_rtti_delete(rettype);
    ctk_rtti_list_delete((void **)stmts);
    return NULL;
}

static eris_node_decl_t *eris_parse_decl(eris_parser_t *parser) {
    ctk_token_t *curr = ctk_parser_curr(parser);

    switch (curr->kind) {
        case ERIS_TOKEN_FUNCTION:
            return eris_parse_function_decl(parser);
        
        default:
            parser->expect_error(curr, 0, "expected declaration");
            return NULL;
    } 
}

eris_node_source_t *eris_parse_file(ctk_span_t *span) {
    eris_parser_t parser;
    eris_parser_init(&parser, span);

    ctk_list_t decls;
    ctk_list_init(&decls, 4);

    bool has_error = false;
    while (!ctk_parser_at_end(&parser)) {
        eris_node_decl_t *decl = eris_parse_decl(&parser);

        if (decl == NULL) {
            has_error = true;
            ctk_parser_advance(&parser);
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
