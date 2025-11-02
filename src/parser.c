#include "parser.h"
#include "token.h"
#include "eris.h"
#include "ctk/text-context-writer.h"
#include <assert.h>

static void eris_parser_expect_error(ctk_token_t *got, 
                                     int expected, char const *msg) {
    assert(got != NULL); // May not be called on synthetic NONE token
    assert(got->kind != 0);
    assert(expected != 0 || msg != NULL);

    ctk_textctx_writer_t writer = {
        .style = &eris_context_style,
        .focus = got,
    };
    
    fprintf(stderr, "%s:%d:%d: " CTK_ANSI_FG_BRIGHT(CTK_ANSI_RED) 
            "error:" CTK_ANSI_RESET, 
            got->src->name, got->pos.line, got->pos.col);
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
