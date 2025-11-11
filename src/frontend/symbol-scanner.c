#include "frontend/symbol-scanner.h"
#include "frontend/symbol.h"
#include "frontend/parser.h"
#include "frontend/token.h"
#include "ctk/token.h"

static void eris_scan_scope(eris_parser_t *p);

static void eris_scan_subscope(eris_parser_t *p) {
    eris_scopelist_push(p->scopes);
    eris_scan_scope(p);

    eris_parser_expect(p, ERIS_TOKEN_RBRACE, NULL);
    eris_scopelist_pop(p->scopes);
}

static void eris_scan_class_declaration(eris_parser_t *p) {
    ctk_token_t *id = eris_parser_expect(p, ERIS_TOKEN_IDENTIFIER, NULL);
    if (id == NULL) {
        return;
    }

    eris_sym_t *sym = ctk_xmalloc(sizeof(eris_sym_t));
    sym->meta = NULL;
    sym->name = id;

    if (eris_symtable_declare(p->scopes->syms, sym)) {
        fprintf(stderr, "declared symbol '%.*s'\n", (int)(id->lexeme.end - id->lexeme.start), id->lexeme.start);
    } else {
        fprintf(stderr, "duplicate symbol error: '%.*s'\n", (int)(id->lexeme.end - id->lexeme.start), id->lexeme.start);
    }
}

static void eris_scan_scope(eris_parser_t *p) {
    while (!eris_parser_at_end(p) 
            && eris_parser_curr(p)->kind != ERIS_TOKEN_RBRACE) {
        if (eris_parser_accept(p, ERIS_TOKEN_LBRACE)) {
            eris_scan_subscope(p);
        } else if (eris_parser_accept(p, ERIS_TOKEN_CLASS)) {
            eris_scan_class_declaration(p);
        } else {
            eris_parser_advance(p);
        }
    }
}

void eris_scan_symbols(ctk_span_t *toks, eris_scopelist_t *scopes) {
    fprintf(stderr, "== start token scan ==\n");
    
    eris_parser_t p;
    eris_parser_init(&p, toks, scopes);

    eris_scopelist_push(scopes);
    eris_scan_scope(&p);

    /* TODO: check result */
    eris_parser_expect(&p, ERIS_TOKEN_EOF, NULL);

    fprintf(stderr, "==  end token span  ==\n");

    eris_scopelist_write(scopes);
}
