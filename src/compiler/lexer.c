#include "compiler/lexer.h"
#include "compiler/logger.h"
#include "util/error.h"
#include "util/alloc.h"
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

typedef struct {
    er_tokkind_t kind;
    char const *lexeme;
} er_toksymbol_t;

#define X(n, l) { ER_TOK_##n, l },
static er_toksymbol_t const er_keyword_tokens[] = {
    ER_KEYWORD_TOKENS(X)
};

static er_toksymbol_t const er_special_tokens[] = {
    ER_SPECIAL_TOKENS(X)
};
#undef X

static size_t const er_n_keyword_tokens 
        = sizeof(er_keyword_tokens) / sizeof(*er_keyword_tokens);

static size_t const er_n_special_tokens
        = sizeof(er_special_tokens) / sizeof(*er_special_tokens);

typedef struct {
    size_t at;
    er_textpos_t pos;
    bool at_eof;
} er_lexstate_t;

typedef struct {
    er_buildmod_t *bmod;

    er_lexstate_t base;
    er_lexstate_t curr;

    er_tok_t *toks;
    size_t toks_size;
    size_t toks_cap;
} er_lexctx_t;

static bool er_is_special_character(char c) {
    switch (c) {
        case '(':
        case ')':
        case '{':
        case '}':
        case '[':
        case ']':
        case ';':
            return true;

        default:
            return false;
    }
}

static void er_lex_err(er_lexctx_t *ctx, char const *msg) {
    char const *s = &ctx->bmod->text[ctx->base.at];
    int len = ctx->curr.at - ctx->base.at;
    er_err(ctx->bmod, ctx->base.pos, "%s: %.*s", msg, len, s);
}

static er_tokkind_t er_lex_lookup_tokkind(er_lexctx_t *ctx, 
                                          er_toksymbol_t const *toks, 
                                          size_t n_toks) {
    char const *s = &ctx->bmod->text[ctx->base.at];
    int len = ctx->curr.at - ctx->base.at;

    for (size_t i = 0; i < n_toks; i++) {
        char const *s2 = toks[i].lexeme;

        if (strncmp(s, s2, len) == 0 && s2[len] == '\0') {
            return toks[i].kind;
        }
    }

    return ER_TOK_NONE;
}

static void er_lex_emit(er_lexctx_t *ctx, er_tokkind_t kind) {
    if (ctx->toks_size + 1 >= ctx->toks_cap) {
        ctx->toks_cap *= 2;
        ctx->toks = er_xrealloc(ctx->toks, ctx->toks_cap * sizeof(er_tok_t));
    }

    er_tok_t *tok = &ctx->toks[ctx->toks_size++];

    tok->kind = kind;
    tok->text.data = ctx->bmod->text + ctx->base.at;
    tok->text.len = ctx->curr.at - ctx->base.at;
    tok->pos = ctx->base.pos;

    ctx->base = ctx->curr;
}

static void er_lex_discard(er_lexctx_t *ctx) {
    ctx->base = ctx->curr;
}

static inline char er_lex_next(er_lexctx_t *ctx) {
    if (ctx->bmod->text[ctx->curr.at] == '\n') {
        ctx->curr.pos.line++;
        ctx->curr.pos.col = 1;
    } else {
        ctx->curr.pos.col++;
    }

    ctx->curr.at++;
    if (ctx->curr.at >= ctx->bmod->size) {
        ctx->curr.at_eof = true;
        return '\0';
    } else {
        return ctx->bmod->text[ctx->curr.at];
    }
}

static void er_lex_identifier(er_lexctx_t *ctx) {
    char c;
    do {
        c = er_lex_next(ctx);
    } while (isalnum(c) || c == '_');

    er_tokkind_t kind = er_lex_lookup_tokkind(ctx, er_keyword_tokens, 
                                              er_n_keyword_tokens);
    if (kind == ER_TOK_NONE) { 
        er_lex_emit(ctx, ER_TOK_IDENTIFIER);
    } else {
        er_lex_emit(ctx, kind);
    }
}

static void er_lex_number(er_lexctx_t *ctx) {
    char c;
    do {
        c = er_lex_next(ctx);
    } while (isalnum(c) || c == '_');

    er_lex_emit(ctx, ER_TOK_NUMBER);
}

static void er_lex_special(er_lexctx_t *ctx) {
    er_tokkind_t kind = ER_TOK_NONE, subkind;
    er_lexstate_t state;

    char c;
    do {
        c = er_lex_next(ctx);

        subkind = er_lex_lookup_tokkind(ctx, er_special_tokens, 
                                        er_n_special_tokens);
        if (subkind != ER_TOK_NONE) {
            kind = subkind;
            state = ctx->curr;
        }
    } while (er_is_special_character(c));

    if (kind == ER_TOK_NONE) {
        er_lex_err(ctx, "unrecognized token");
        er_lex_discard(ctx);
    } else {
        ctx->curr = state;
        er_lex_emit(ctx, kind);
    }
}

static void er_lex_comment(er_lexctx_t *ctx) {
    char c;
    do {
        c = er_lex_next(ctx);
    } while (c != '\n' || ctx->curr.at_eof);

    er_lex_discard(ctx);
}

er_tok_t *er_lex(er_buildmod_t *bmod) {
    er_lexctx_t ctx = {
        .bmod = bmod,
    };

    ctx.base.at = 0;
    ctx.base.pos.line = ctx.base.pos.col = 1;

    ctx.curr = ctx.base;

    ctx.toks_cap = 128;
    ctx.toks = er_xmalloc(ctx.toks_cap * sizeof(er_tok_t));

    while (ctx.curr.at < ctx.bmod->size) {
        char c = ctx.bmod->text[ctx.curr.at];

        if (isalpha(c) || c == '_') {
            er_lex_identifier(&ctx);
        } else if (isalnum(c)) {
            er_lex_number(&ctx);
        } else if (er_is_special_character(c)) {
            er_lex_special(&ctx);
        } else if (c == '#') {
            er_lex_comment(&ctx);
        } else if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            er_lex_next(&ctx);
            er_lex_discard(&ctx);
        } else {
            er_lex_next(&ctx);
            er_lex_err(&ctx, "unrecognized character");
            er_lex_discard(&ctx);
        }
    }

    er_lex_emit(&ctx, ER_TOK_ENDOFINPUT);

    return ctx.toks;
}
