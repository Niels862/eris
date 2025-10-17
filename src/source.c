#include "source.h"
#include "lexer.h"
#include "file-parser.h"

void eris_src_init(eris_src_t *src, ctk_zstr_t filename, FILE *file) {
    ctk_textsrc_init_file(&src->textsrc, filename, file);
    ctk_tokenlist_init(&src->toks);
    src->root = NULL;
}

void eris_src_destruct(eris_src_t *src) {
    ctk_rtti_delete(src->root);
    ctk_tokenlist_destruct(&src->toks);
    ctk_textsrc_destruct(&src->textsrc);
}

void eris_src_lex(eris_src_t *src) {
    eris_lex(&src->textsrc, &src->toks);

    fprintf(stderr, "Lexed %ld tokens:\n", src->toks.size);
    for (size_t i = 0; i < src->toks.size; i++) {
        fprintf(stderr, "[%ld] ", i);
        ctk_token_write(&src->toks.data[i], stderr);
        fprintf(stderr, "\n");
    }
}

void eris_src_parse_file(eris_src_t *src) {
    ctk_span_t span;
    ctk_tokenlist_to_span(&src->toks, &span);

    src->root = eris_parse_file(&span);

    ctk_rtti_write(src->root, 0, stderr);
    fprintf(stderr, "\n");
}
