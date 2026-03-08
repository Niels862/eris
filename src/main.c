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

    int ret = 1;
    er_buildmod_t *bmod = er_buildmod_read(argv[1]);
    if (bmod == NULL) {
        fprintf(stderr, "error reading module: %s\n", argv[1]);
        goto end;
    }

    er_lex(bmod);
    if (bmod->toks == NULL) {
        goto end;
    }

    er_parse(bmod);
    if (bmod->root == NULL) {
        goto end;
    }

    er_ast_print(bmod->root);

    er_astmod_t *Mod = &bmod->root->data.Mod;
    for (size_t i = 0; i < Mod->n_funcs; i++) {
        er_astnode_t *funcnode = Mod->funcs[i];
        er_str_t *name = &funcnode->data.Func.name;

        er_buildfunc_t *bfunc = er_buildfunc_new(bmod, funcnode);

        fprintf(stderr, "Lowering '%.*s'...\n", name->len, name->data);
        er_lower(bmod, bfunc);
    }    

    ret = 0;

end:
    er_buildmod_delete(bmod);
    return ret;
}
