#include "lexer.h"
#include "token.h"
#include <ctype.h>
#include <stdbool.h>
#include <assert.h>

static ctk_fixed_lexeme_t eris_keywords[] = {
    ERIS_TOKENS_KEYWORD(ERIS_X_EXPAND_FIXED)
    { 0 }
};

static ctk_fixed_lexeme_t eris_separators[] = {
    ERIS_TOKENS_SEPARATOR(ERIS_X_EXPAND_FIXED)
    { 0 }
};

static ctk_fixed_lexeme_t eris_operators[] = {
    ERIS_TOKENS_OPERATOR(ERIS_X_EXPAND_FIXED)
    { 0 }
};

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

static bool eris_is_number(ctk_lexer_t *lexer) {
    uint32_t c = lexer->curr;
    return isdigit(c) || c == '_';
}

static bool eris_is_separator(ctk_lexer_t *lexer) {
    switch (lexer->curr) {
        case '{':
        case '}':
        case '(':
        case ')':
        case '[':
        case ']':
        case ';':
        case ':':
            return true;

        default:
            return false;
    }
}

static bool eris_is_operator(ctk_lexer_t *lexer) {
    switch (lexer->curr) {
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case '<':
        case '>':
        case '=':
        case '~':
        case '|':
        case '&':
        case '?':
        case '!':
        case '@':
            return true;

        default:
            return false;
    }
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

static void eris_lex_identifier(ctk_lexer_t *lexer, ctk_tokenlist_t *toks) {
    do {
        ctk_lexer_advance(lexer);
    } while (eris_is_identifier_continue(lexer));

    eris_tokenkind_t kind = ctk_lexer_lookup(lexer, eris_keywords);
    if (kind == ERIS_TOKEN_NULL) {
        kind = ERIS_TOKEN_IDENTIFIER;
    }

    eris_emit(lexer, toks, kind);
}

static void eris_lex_number(ctk_lexer_t *lexer, ctk_tokenlist_t *toks) {
    do {
        ctk_lexer_advance(lexer);
    } while (eris_is_number(lexer));

    eris_emit(lexer, toks, ERIS_TOKEN_NUMBER);
}

static void eris_lex_separator(ctk_lexer_t *lexer, ctk_tokenlist_t *toks) {
    ctk_lexer_advance(lexer);

    eris_tokenkind_t kind = ctk_lexer_lookup(lexer, eris_separators);
    assert(kind != ERIS_TOKEN_NULL);

    eris_emit(lexer, toks, kind);
}

static void eris_lex_operator(ctk_lexer_t *lexer, ctk_tokenlist_t *toks) {
    ctk_lexer_state_t state;
    eris_tokenkind_t kind = ERIS_TOKEN_UNRECOGNIZED;

    do {
        ctk_lexer_advance(lexer);

        eris_tokenkind_t lkind = ctk_lexer_lookup(lexer, eris_operators);
        if (lkind != ERIS_TOKEN_NULL) {
            ctk_lexer_save_state(lexer, &state);
            kind = lkind;
        }
    } while (eris_is_operator(lexer));

    if (kind != ERIS_TOKEN_UNRECOGNIZED) {
        ctk_lexer_restore_state(lexer, &state);
    }
    
    eris_emit(lexer, toks, kind);
}

static void eris_lex_whitespace(ctk_lexer_t *lexer) {
    do {
        ctk_lexer_advance(lexer);
    } while (eris_is_whitespace(lexer));

    ctk_lexer_discard(lexer);
}

static void eris_lex_comment(ctk_lexer_t *lexer) {
    do {
        ctk_lexer_advance(lexer);
    } while (eris_is_comment_continue(lexer));

    ctk_lexer_discard(lexer);
}

void eris_lex(ctk_textsrc_t *ts, ctk_tokenlist_t *toks) {
    ctk_lexer_t lexer;
    ctk_lexer_init(&lexer, ts, ctk_decode_raw);

    do {
        if (eris_is_identifier_start(&lexer)) {
            eris_lex_identifier(&lexer, toks);
        } else if (eris_is_number(&lexer)) {
            eris_lex_number(&lexer, toks);
        } else if (eris_is_separator(&lexer)) {
            eris_lex_separator(&lexer, toks);
        } else if (eris_is_operator(&lexer)) {
            eris_lex_operator(&lexer, toks);
        } else if (eris_is_whitespace(&lexer)) {
            eris_lex_whitespace(&lexer);
        } else if (eris_is_comment_start(&lexer)) {
            eris_lex_comment(&lexer);
        } else {
            ctk_lexer_advance(&lexer);
            eris_emit(&lexer, toks, ERIS_TOKEN_UNRECOGNIZED);
        }
    } while (!ctk_lexer_at_eof(&lexer));

    eris_emit(&lexer, toks, ERIS_TOKEN_EOF);

    ctk_tokenlist_lock(toks);
}
