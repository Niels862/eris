#include "file-parser.h"
#include "ctk/parser.h"
#include <assert.h>

static eris_node_decl_t *eris_parse_function_decl(ctk_parser_t *parser) {
    ctk_parser_expect(parser, ERIS_TOKEN_FUNCTION);

    ctk_token_t *name = ctk_parser_expect(parser, ERIS_TOKEN_IDENTIFIER);

    ctk_parser_expect(parser, ERIS_TOKEN_LPAREN);
    ctk_parser_expect(parser, ERIS_TOKEN_RPAREN);

    ctk_parser_expect(parser, ERIS_TOKEN_LBRACE);
    ctk_parser_expect(parser, ERIS_TOKEN_RBRACE);

    return eris_node_function_decl_new(name);
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

    while (!ctk_parser_accept(&parser, ERIS_TOKEN_EOF)) {
        eris_node_decl_t *decl = eris_parse_decl(&parser);

        if (decl == NULL) {
            ctk_parser_advance(&parser);
        } else {
            ctk_list_add(&decls, decl);
        }
    }

    /* Guaranteed by lexer. */
    assert(ctk_parser_at_end(&parser));

    return stem_node_source_new(&decls);
}
