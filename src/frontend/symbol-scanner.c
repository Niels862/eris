#include "frontend/symbol-scanner.h"
#include "frontend//parser.h"
#include "frontend/token.h"
#include "ctk/token.h"

void eris_scan_scope(eris_parser_t *p, eris_scopelist_t *scopes);

void eris_scan_subscope(ctk_parser_t *p, eris_scopelist_t *scopes) {
    eris_scopelist_push(scopes);
    eris_scan_scope(p, scopes);

    ctk_parser_expect(p, ERIS_TOKEN_RBRACE, NULL);
    eris_scopelist_pop(scopes);
}

void eris_scan_class_declaration(ctk_parser_t *p, eris_scopelist_t *scopes) {
    (void)scopes;

    ctk_token_t *id = ctk_parser_expect(p, ERIS_TOKEN_IDENTIFIER, NULL);
    if (id == NULL) {
        return;
    }

    /* Temporarily insert `1` instead of NULL to check for previous insertion,
       which would otherwise be NULL as well. */
    void *prev = ctk_linmap_insert(&scopes->syms->map, &id->lexeme, (void *)1);
    if (prev != NULL) {
        fprintf(stderr, "duplicate symbol error: '%.*s'\n", (int)(id->lexeme.end - id->lexeme.start), id->lexeme.start);
        // TODO: duplicate symbol error
    } else {
        fprintf(stderr, "declared symbol '%.*s' (prev = %p)\n", (int)(id->lexeme.end - id->lexeme.start), id->lexeme.start, prev);
    }
}

void eris_scan_scope(eris_parser_t *p, eris_scopelist_t *scopes) {
    while (!ctk_parser_at_end(p) && p->curr->kind != ERIS_TOKEN_RBRACE) {
        if (ctk_parser_accept(p, ERIS_TOKEN_LBRACE)) {
            eris_scan_subscope(p, scopes);
        } else if (ctk_parser_accept(p, ERIS_TOKEN_CLASS)) {
            eris_scan_class_declaration(p, scopes);
        } else {
            ctk_parser_advance(p);
        }
    }
}

void eris_scan_symbols(ctk_span_t *toks, eris_scopelist_t *scopes) {
    fprintf(stderr, "== start token scan ==\n");
    
    eris_parser_t parser;
    eris_parser_init(&parser, toks);

    eris_scopelist_push(scopes);
    eris_scan_scope(&parser, scopes);

    /* TODO: check result */
    ctk_parser_expect(&parser, ERIS_TOKEN_EOF, NULL);

    fprintf(stderr, "==  end token span  ==\n");

    eris_scopelist_write(scopes);
}
