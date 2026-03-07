#include "compiler/lex.h"
#include "compiler/parse.h"
#include "compiler/lower.h"
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

    er_astnode_t *rootnode = er_parse(bmod, toks);
    er_ast_print(rootnode);
    free(toks);

    er_astmod_t *Mod = &rootnode->data.Mod;
    for (size_t i = 0; i < Mod->n_funcs; i++) {
        er_astnode_t *funcnode = Mod->funcs[i];
        er_str_t *name = &funcnode->data.Func.name;

        fprintf(stderr, "Lowering '%.*s'...\n", name->len, name->data);
        er_lower(bmod, funcnode);
    }    

    er_buildmod_delete(bmod);

    return 0;
}
