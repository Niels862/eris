#include "parser.h"
#include "token.h"
#include "eris.h"
#include "ctk/text-context-writer.h"
#include <assert.h>

static ctk_token_t eris_synthetic_after_token(ctk_token_t *tok) {
    ctk_token_t *before = tok - 1;

    // BEFORE is synthetic token (STARTSOURCE) -> give up
    if (before->src->name == NULL) {
        return *tok;
    }

    ctk_token_t after = *before;

    // FIXME: needs to account for encoding and newlines
    after.pos.col += after.lexeme.end - after.lexeme.start;
    after.lexeme.start = after.lexeme.end;

    return after;
}

static void eris_parser_expect_error(ctk_token_t *got, 
                                     int expected, char const *msg) {
    assert(got != NULL); // May not be called on synthetic NONE token
    assert(got->kind != 0);
    assert(expected != 0 || msg != NULL);

    ctk_token_t tok;
    if (got->kind == ERIS_TOKEN_EOF || expected == ERIS_TOKEN_SEMICOLON) {
        tok = eris_synthetic_after_token(got);
    } else {
        tok = *got;
    }

    ctk_textctx_writer_t writer = {
        .style = &eris_context_style,
        .focus = &tok,
    };
    
    fprintf(stderr, "%s:%d:%d: " CTK_ANSI_FG_BRIGHT(CTK_ANSI_RED) 
            "error:" CTK_ANSI_RESET, 
            tok.src->name, tok.pos.line, tok.pos.col);
    if (msg != NULL) {
        fprintf(stderr, " %s", msg);
    } else {
        fprintf(stderr, " expected %s", ctk_tokenkind_get_name(expected));
    }

    if (got->kind == ERIS_TOKEN_EOF) {
        fprintf(stderr, " before end of input\n");
    } else {
        fprintf(stderr, "\n");
    }

    ctk_textctx_write(&writer);
}

void eris_parser_init(eris_parser_t *parser, ctk_span_t *span) {
    ctk_parser_init(parser, span, &eris_parser_expect_error);
}
