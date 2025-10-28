#include "file-parser.h"
#include "ctk/parser.h"
#include <assert.h>

// FIXME: overflow, errors, ...
static int64_t eris_parse_number(ctk_token_t *num) {
    int64_t s64 = 0;

    for (char *s = num->lexeme.start; s != num->lexeme.end; s++) {
        s64 = s64 * 10 + (*s - '0');
    }

    return s64;
}

static eris_node_expr_t *eris_parse_atom(ctk_parser_t *parser) {
    ctk_token_t *value = ctk_parser_expect(parser, ERIS_TOKEN_NUMBER);
    return eris_node_intlit_new(value, eris_parse_number(value));
}

static eris_node_expr_t *eris_parse_expr(ctk_parser_t *parser) {
    return eris_parse_atom(parser);
}

static eris_node_stmt_t *eris_parse_return(ctk_parser_t *parser) {
    ctk_token_t *token = ctk_parser_expect(parser, ERIS_TOKEN_RETURN);

    eris_node_expr_t *value = eris_parse_expr(parser);
    if (value == NULL || !ctk_parser_expect(parser, ERIS_TOKEN_SEMICOLON)) {
        ctk_rtti_delete(value);
        return NULL;
    }

    return eris_node_return_new(token, value);
}

static eris_node_stmt_t *eris_parse_expr_stmt(ctk_parser_t *parser) {
    eris_node_expr_t *expr = eris_parse_expr(parser);
    if (expr == NULL) {
        return NULL;
    }

    ctk_parser_expect(parser, ERIS_TOKEN_SEMICOLON);
    
    return eris_node_expr_stmt_new(expr);
}

static eris_node_stmt_t *eris_parse_stmt(ctk_parser_t *parser) {
    switch (ctk_parser_curr(parser)->kind) {
        case ERIS_TOKEN_RETURN:     return eris_parse_return(parser);
        default:                    return eris_parse_expr_stmt(parser);
    }
}

static eris_node_stmt_t **eris_parse_stmt_body(ctk_parser_t *parser) {
    ctk_parser_expect(parser, ERIS_TOKEN_LBRACE);

    ctk_list_t stmts;
    ctk_list_init(&stmts, 4);

    while (!ctk_parser_accept(parser, ERIS_TOKEN_RBRACE)) {
        eris_node_stmt_t *stmt = eris_parse_stmt(parser);

        if (stmt == NULL || ctk_parser_at_end(parser)) {
            ctk_rtti_delete(stmt);
            ctk_rtti_list_delete(ctk_list_move(&stmts));
            return NULL;
        }

        ctk_list_add(&stmts, stmt);
    }

    return (eris_node_stmt_t **)ctk_list_move(&stmts);
}

static eris_node_decl_t *eris_parse_function_decl(ctk_parser_t *parser) {
    ctk_parser_expect(parser, ERIS_TOKEN_FUNCTION);

    ctk_token_t *name = ctk_parser_expect(parser, ERIS_TOKEN_IDENTIFIER);

    ctk_parser_expect(parser, ERIS_TOKEN_LPAREN);
    ctk_parser_expect(parser, ERIS_TOKEN_RPAREN);

    eris_node_stmt_t **stmts = eris_parse_stmt_body(parser);
    if (stmts == NULL) {
        return NULL;
    }

    return eris_node_function_decl_new(name, stmts);
}

static eris_node_decl_t *eris_parse_decl(ctk_parser_t *parser) {
    switch (ctk_parser_curr(parser)->kind) {
        case ERIS_TOKEN_FUNCTION:
            return eris_parse_function_decl(parser);
        
        default:
            fprintf(stderr, "expected declaration\n");
            return NULL;
    } 
}

eris_node_source_t *eris_parse_file(ctk_span_t *span) {
    ctk_parser_t parser;
    ctk_parser_init(&parser, span);

    ctk_list_t decls;
    ctk_list_init(&decls, 4);

    ctk_parser_expect(&parser, CTK_TOKEN_STARTSOURCE);

    while (!ctk_parser_accept(&parser, ERIS_TOKEN_EOF)) {
        eris_node_decl_t *decl = eris_parse_decl(&parser);

        if (decl == NULL) {
            ctk_parser_advance(&parser);
        } else {
            ctk_list_add(&decls, decl);
        }
    }

    ctk_parser_expect(&parser, CTK_TOKEN_ENDSOURCE);
    assert(ctk_parser_at_end(&parser));
    
    return eris_node_source_new(&decls);
}
