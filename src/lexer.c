#include "lexer.h"
#include "token.h"
#include <ctype.h>
#include <stdbool.h>

static void eris_emit(ctk_lexer_t *lexer, ctk_tokenlist_t *toks, 
                      eris_tokenkind_t kind) {
    static ctk_token_t tok;

    ctk_lexer_emit(lexer, &tok, kind);
    ctk_tokenlist_add(toks, &tok);
}

static bool eris_is_identifier_start(ctk_lexer_t *lexer) {
    uint32_t c = lexer->curr;
    return isalpha(c) || c == '_';
}

static bool eris_is_identifier_continue(ctk_lexer_t *lexer) {
    uint32_t c = lexer->curr;
    return isalnum(c) || c == '_';
}

static bool eris_is_whitespace(ctk_lexer_t *lexer) {
    uint32_t c = lexer->curr;
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static bool eris_is_comment_start(ctk_lexer_t *lexer) {
    return lexer->curr == '#';
}

static bool eris_is_comment_continue(ctk_lexer_t *lexer) {
    uint32_t c = lexer->curr;
    return isprint(c) && c != '\n';
}

void eris_lex(ctk_textsrc_t *ts, ctk_tokenlist_t *toks) {
    ctk_lexer_t lexer;
    ctk_lexer_init(&lexer, ts, ctk_decode_raw);

    do {
        if (eris_is_identifier_start(&lexer)) {
            do {
                ctk_lexer_advance(&lexer);
            } while (eris_is_identifier_continue(&lexer));

            eris_emit(&lexer, toks, ERIS_TOKEN_IDENTIFIER);
        } else if (eris_is_whitespace(&lexer)) {
            do {
                ctk_lexer_advance(&lexer);
            } while (eris_is_whitespace(&lexer));

            ctk_lexer_discard(&lexer);
        } else if (eris_is_comment_start(&lexer)) {
            do {
                ctk_lexer_advance(&lexer);
            } while (eris_is_comment_continue(&lexer));

            ctk_lexer_discard(&lexer);
        } else {
            ctk_lexer_advance(&lexer);
            eris_emit(&lexer, toks, ERIS_TOKEN_UNRECOGNIZED);
        }
    } while (!ctk_lexer_at_eof(&lexer));

    eris_emit(&lexer, toks, ERIS_TOKEN_EOF);
}
