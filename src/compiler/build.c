#include "compiler/build.h"
#include "compiler/lex.h"
#include "compiler/parse.h"
#include "compiler/irgen.h"
#include "util/file.h"
#include "util/alloc.h"
#include "util/string.h"
#include "util/error.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define ER_PATH_BUFFER_SIZE 4096

er_mod_t **er_build(char const *entry) {
    er_mod_t **mods = NULL;

    er_buildmod_t *bmod = er_buildmod_read(entry);
    if (bmod == NULL) {
        fprintf(stderr, "could not load entry module: '%s'\n", entry);
        return NULL;
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
        er_irgen(bmod, bfunc);

        er_buildfunc_delete(bfunc);
    }    

end:
    er_buildmod_delete(bmod);
    return mods;
}

static er_buildmod_t *er_buildmod_new(char const *filename, 
                                      char *text, size_t size) {
    er_buildmod_t *bmod = er_xmalloc(sizeof(er_buildmod_t));

    bmod->text = text;
    bmod->size = size;
    bmod->filename = er_strdup(filename);

    bmod->arenas.persistent = er_arena_new(4096);
    bmod->arenas.parse = er_arena_new(4096);

    bmod->toks = NULL;
    bmod->root = NULL;

    return bmod;
}

void er_buildmod_delete(er_buildmod_t *bmod) {
    free(bmod->text);
    free(bmod->filename);
    
    er_arena_delete(bmod->arenas.persistent);
    er_arena_delete(bmod->arenas.parse);

    if (bmod->toks != NULL) {
        free(bmod->toks);
    }

    free(bmod);
}

static er_buildmod_t *er_buildmod_read_path(char const *path) {
    char *text;
    size_t size;
    if (!er_read_text_file(path, &text, &size)) {
        return NULL;
    }

    return er_buildmod_new(path, text, size);
}

er_buildmod_t *er_buildmod_read(char const *module) {
    char const *paths[] = {
        "testdata/"
    };

    for (size_t i = 0; i < sizeof(paths) / sizeof(*paths); i++) {
        char buf[ER_PATH_BUFFER_SIZE];

        strcpy(buf, paths[i]);
        strcat(buf, module);
        strcat(buf, ".eris");

        if (er_is_file(buf)) {
            return er_buildmod_read_path(buf);
        }
    }

    return NULL;
}

er_buildfunc_t *er_buildfunc_new(er_buildmod_t *bmod, er_astnode_t *funcnode) {
    assert(funcnode->kind == ER_AST_FUNC);

    er_buildfunc_t *bfunc = er_arena_alloc(bmod->arenas.persistent, 
                                           sizeof(er_buildfunc_t));
    
    bfunc->root = funcnode;
    bfunc->blocks = NULL;
    bfunc->n_blocks = 0;

    bfunc->arenas.ir = er_arena_new(256);

    return bfunc;
}

void er_buildfunc_delete(er_buildfunc_t *bfunc) {
    er_arena_delete(bfunc->arenas.ir);
}