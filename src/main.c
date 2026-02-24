#include "compiler/lexer.h"
#include "util/file.h"
#include "util/error.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s [filename]\n", argv[0]);
        return 1;
    }

    char const *filename = argv[1];

    char *text;
    size_t size;
    
    if (!er_read_text_file(filename, &text, &size)) {
        fprintf(stderr, "could not read file: %s\n", filename);
        return 1;
    }

    er_buildmod_t bmod = {
        .filename   = filename,
        .text       = text,
        .size       = size,
    };

    er_tok_t *toks = er_lex(&bmod);
    if (toks == NULL) {
        free(text);
        return 1;
    }

    free(text);
    free(toks);
    return 0;
}
