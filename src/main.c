#include "compiler/lexer.h"
#include "util/file.h"
#include "util/error.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    ER_UNUSED(argc), ER_UNUSED(argv);

    char const *filename = "design.h";

    char *text;
    size_t size;
    
    if (!er_read_text_file(filename, &text, &size)) {
        fprintf(stderr, "could not read file: %s\n", filename);
        return 1;
    }

    er_tok_t *toks = er_lex(filename, text, size);
    if (toks == NULL) {
        free(text);
        return 1;
    }

    size_t i = 0;
    do {
        fprintf(stderr, "%s:", filename);
        er_tok_print(&toks[i], stderr);
    } while (toks[i++].kind != ER_TOK_ENDOFINPUT);

    free(text);
    free(toks);
    return 0;
}
