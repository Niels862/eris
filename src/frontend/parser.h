#ifndef ERIS_PARSER_H
#define ERIS_PARSER_H

#include "frontend/token.h"
#include "frontend/symbol-table.h"
#include "ctk/parser.h"

typedef struct {
    ctk_parser_t base;
    eris_scopelist_t *scopes;
} eris_parser_t;

void eris_parser_init(eris_parser_t *parser, ctk_span_t *span, 
                      eris_scopelist_t *scopes);

static inline ctk_token_t *eris_parser_accept(eris_parser_t *p, 
                                              eris_tokenkind_t kind) {
    return ctk_parser_accept(&p->base, kind);
}

static inline ctk_token_t *eris_parser_expect(eris_parser_t *p, 
                                              eris_tokenkind_t kind, 
                                              char const *msg) {
    return ctk_parser_expect(&p->base, kind, msg);
}

static inline void eris_parser_advance(eris_parser_t *p) {
    ctk_parser_advance(&p->base);
}

static inline ctk_token_t *eris_parser_curr(eris_parser_t *p) {
    return p->base.curr;
}

static inline bool eris_parser_at_end(eris_parser_t *p) {
    return ctk_parser_at_end(&p->base);
}

static inline void eris_parser_expect_error(eris_parser_t *p, 
                                            ctk_token_t *got, 
                                            eris_tokenkind_t expected, 
                                            char const *msg) {
    p->base.expect_error(got, expected, msg);
}

#endif
