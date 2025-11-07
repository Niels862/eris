#include "frontend/structural-parser.h"
#include "frontend/parser.h"

void eris_parse_body(eris_parser_t *p, ctk_list_t *stmts);

bool eris_is_skeleton_token(ctk_parser_t *p) {
    if (ctk_parser_at_end(p)) {
        return false;
    }

    if (p->curr->kind == ERIS_TOKEN_LBRACE 
            || p->curr->kind == ERIS_TOKEN_RBRACE) {
        return false;
    }

    return true;
}

eris_node_stmt_t *eris_parse_skeleton(ctk_parser_t *p) {
    ctk_token_t *start = p->curr;

    while (eris_is_skeleton_token(p)) {
        ctk_parser_advance(p);
    }

    ctk_span_t toks;
    ctk_span_init(&toks, start, p->curr);

    eris_node_stmt_t **stmts = NULL;
    if (ctk_parser_accept(p, ERIS_TOKEN_LBRACE)) {
        ctk_list_t list;
        ctk_list_init(&list, 4);

        eris_parse_body(p, &list);

        stmts = (eris_node_stmt_t **)ctk_list_move(&list);

        ctk_parser_expect(p, ERIS_TOKEN_RBRACE, NULL);
    }

    return eris_node_skel_new(&toks, stmts);
}

void eris_parse_body(eris_parser_t *p, ctk_list_t *stmts) {    
    while (!ctk_parser_at_end(p) && p->curr->kind != ERIS_TOKEN_RBRACE) {
        ctk_list_add(stmts, eris_parse_skeleton(p));
    }
}

eris_node_source_t *eris_parse_structure(ctk_span_t *toks) {
    eris_parser_t p;
    eris_parser_init(&p, toks);

    ctk_list_t stmts;
    ctk_list_init(&stmts, 4);

    eris_parse_body(&p, &stmts);

    if (!ctk_parser_at_end(&p)) {
        p.expect_error(p.curr, 0, "no matching '{'");
    }

    return eris_node_source_new(&stmts);
}
