#include "source.h"
#include "lexer.h"
#include "file-parser.h"

void eris_codesrc_init(eris_codesrc_t *csrc, ctk_zstr_t filename, FILE *file) {
    ctk_textsrc_init_file(&csrc->textsrc, filename, file);
    ctk_tokenlist_init(&csrc->toks);
    csrc->root = NULL;
}

void eris_codesrc_destruct(eris_codesrc_t *csrc) {
    ctk_rtti_delete(csrc->root);
    ctk_tokenlist_destruct(&csrc->toks);
    ctk_textsrc_destruct(&csrc->textsrc);
}

void eris_codesrc_lex(eris_codesrc_t *csrc) {
    eris_lex(&csrc->textsrc, &csrc->toks);

    fprintf(stderr, "Lexed %ld tokens:\n", csrc->toks.size);
    for (size_t i = 0; i < csrc->toks.size; i++) {
        fprintf(stderr, "[%ld] ", i);
        ctk_token_write(&csrc->toks.data[i], stderr);
        fprintf(stderr, "\n");
    }
}

void eris_codesrc_parse_file(eris_codesrc_t *csrc) {
    ctk_span_t span;
    ctk_tokenlist_to_span(&csrc->toks, &span);

    csrc->root = eris_parse_file(&span);

    ctk_rtti_write(csrc->root, 0, stderr);
    fprintf(stderr, "\n");
}
