#include "compiler/lexer.h"
#include "util/error.h"
#include "util/alloc.h"
#include <ctype.h>

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
} er_lexstate_t;

typedef struct {
    char const *filename;
    char const *text;
    size_t const size;

    er_lexstate_t base;
    er_lexstate_t curr;

    er_tok_t *toks;
    size_t toks_size;
    size_t toks_cap;
} er_lexctx_t;

void er_lex_emit(er_lexctx_t *ctx, er_tokkind_t kind) {
    if (ctx->toks_size + 1 >= ctx->toks_cap) {
        ctx->toks_cap *= 2;
        ctx->toks = er_xrealloc(ctx->toks, ctx->toks_cap * sizeof(er_tok_t));
    }

    er_tok_t *tok = &ctx->toks[ctx->toks_size++];

    tok->kind = kind;
    tok->text.data = ctx->text + ctx->base.at;
    tok->text.len = ctx->curr.at - ctx->base.at;
    tok->pos = ctx->base.pos;

    ctx->base = ctx->curr;
}

static void er_lex_discard(er_lexctx_t *ctx) {
    ctx->base = ctx->curr;
}

static inline char er_lex_next(er_lexctx_t *ctx) {
    if (ctx->text[ctx->curr.at] == '\n') {
        ctx->curr.pos.line++;
        ctx->curr.pos.col = 1;
    } else {
        ctx->curr.pos.col++;
    }

    return ctx->text[++ctx->curr.at];
}

er_tok_t *er_lex(char const *filename, char const *text, size_t size) {
    ER_UNUSED(er_n_keyword_tokens), ER_UNUSED(er_n_special_tokens);

    er_lexctx_t ctx = {
        .filename   = filename,
        .text       = text,
        .size       = size,
    };

    ctx.base.at = 0;
    ctx.base.pos.line = ctx.base.pos.col = 1;

    ctx.curr = ctx.base;

    ctx.toks_cap = 128;
    ctx.toks = er_xmalloc(ctx.toks_cap * sizeof(er_tok_t));

    while (ctx.curr.at < ctx.size) {
        char c = ctx.text[ctx.curr.at];

        if (isalpha(c) || c == '_') {
            do {
                c = er_lex_next(&ctx);
            } while (isalnum(c) || c == '_');

            er_lex_emit(&ctx, ER_TOK_IDENTIFIER);
        } else if (isalnum(c)) {
            do {
                c = er_lex_next(&ctx);
            } while (isalnum(c) || c == '_');

            er_lex_emit(&ctx, ER_TOK_NUMBER);
        } else {
            er_lex_next(&ctx);
            er_lex_discard(&ctx);
        }
    }

    er_lex_emit(&ctx, ER_TOK_ENDOFINPUT);

    return ctx.toks;
}
