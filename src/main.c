#include "compiler/lexer.h"
#include "compiler/parser.h"
#include "util/file.h"
#include "util/error.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s [filename]\n", argv[0]);
        return 1;
    }

    er_buildmod_t *bmod = er_buildmod_read(argv[1]);
    if (bmod == NULL) {
        fprintf(stderr, "error reading module: %s\n", argv[1]);
        return 1;
    }

    er_tok_t *toks = er_lex(bmod);
    if (toks == NULL) {
        return 1;
    }

    er_astnode_t *n = er_parse(bmod, toks);

    ER_UNUSED(n);

    free(toks);
    return 0;
}
