#include "compiler/build.h"
#include "compiler/lex.h"
#include "compiler/parse.h"
#include "compiler/irgen.h"
#include "compiler/declare.h"
#include "compiler/analyze.h"
#include "compiler/codegen.h"
#include "util/file.h"
#include "util/alloc.h"
#include "util/string.h"
#include "util/error.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define ER_PATH_BUFFER_SIZE 4096

static void er_buildfunc_init(er_buildfunc_t *bfunc, er_astnode_t *funcnode) {
    assert(funcnode->kind == ER_AST_FUNC);
    
    bfunc->root = funcnode;
    bfunc->name = &funcnode->data.Func.name;

    bfunc->arenas.ir = er_arena_new(256);

    bfunc->blocks = NULL;
    bfunc->entry = NULL;
    bfunc->n_blocks = 0;

    bfunc->n_temps = 0;
}

static void er_buildfunc_destruct(er_buildfunc_t *bfunc) {
    er_arena_delete(bfunc->arenas.ir);
}

static er_buildmod_t *er_buildmod_new(er_buildctx_t *bctx, 
                                      char const *filename, 
                                      char *text, size_t size) {
    er_buildmod_t *bmod = er_xmalloc(sizeof(er_buildmod_t));

    bmod->text = text;
    bmod->size = size;
    bmod->filename = er_strdup(filename);

    bmod->arenas.persistent = er_arena_new(4096);
    bmod->arenas.parse = er_arena_new(4096);

    bmod->toks = NULL;
    bmod->root = NULL;

    er_symtab_init(&bmod->globals);
    bmod->globals.enclosing = &bctx->builtins;

    bmod->bfuncs = NULL;
    bmod->n_bfuncs = 0;

    bmod->mod = NULL;

    return bmod;
}

static er_buildmod_t *er_buildmod_read_path(er_buildctx_t *bctx, 
                                            char const *path) {
    char *text;
    size_t size;
    if (!er_read_text_file(path, &text, &size)) {
        return NULL;
    }

    return er_buildmod_new(bctx, path, text, size);
}

static er_buildmod_t *er_buildmod_read(er_buildctx_t *bctx, 
                                       char const *module) {
    char const *paths[] = {
        "testdata/"
    };

    for (size_t i = 0; i < sizeof(paths) / sizeof(*paths); i++) {
        char buf[ER_PATH_BUFFER_SIZE];

        strcpy(buf, paths[i]);
        strcat(buf, module);
        strcat(buf, ".eris");

        if (er_is_file(buf)) {
            return er_buildmod_read_path(bctx, buf);
        }
    }

    return NULL;
}

static void er_buildmod_delete(er_buildmod_t *bmod) {
    free(bmod->text);
    free(bmod->filename);
    
    er_arena_delete(bmod->arenas.persistent);
    er_arena_delete(bmod->arenas.parse);

    if (bmod->toks != NULL) {
        free(bmod->toks);
    }

    if (bmod->bfuncs != NULL) {
        for (size_t i = 0; i < bmod->n_bfuncs; i++) {
            er_buildfunc_destruct(&bmod->bfuncs[i]);
        }
        free(bmod->bfuncs);
    }

    er_symtab_destruct(&bmod->globals);

    free(bmod);
}

static void er_buildctx_init(er_buildctx_t *bctx) {
    bctx->arenas.persistent = er_arena_new(256);

    er_symtab_init(&bctx->builtins);

    memset(&bctx->sym, 0, sizeof(bctx->sym));
}

static void er_buildctx_destruct(er_buildctx_t *bctx) {
    er_arena_delete(bctx->arenas.persistent);

    er_symtab_destruct(&bctx->builtins);
}

static void er_create_buildfuncs(er_buildmod_t *bmod) {
    er_astmod_t *Mod = &bmod->root->data.Mod;
    bmod->n_bfuncs = Mod->n_funcs;
    bmod->bfuncs = er_xmalloc(bmod->n_bfuncs * sizeof(er_buildfunc_t));

    for (size_t i = 0; i < Mod->n_funcs; i++) {
        er_astnode_t *funcnode = Mod->funcs[i];
        er_buildfunc_init(&bmod->bfuncs[i], funcnode);
    }
}

er_mod_t **er_build(char const *entry) {
    er_buildctx_t bctx;
    er_buildctx_init(&bctx);

    er_load_builtins(&bctx);

    er_mod_t **mods = NULL;

    er_buildmod_t *bmod = er_buildmod_read(&bctx, entry);
    if (bmod == NULL) {
        fprintf(stderr, "could not load entry module: '%s'\n", entry);
        goto end;
    }

    if (!er_lex(bmod)) {
        goto end;
    }

    if (!er_parse(bmod)) {
        goto end;
    }

    er_ast_print(bmod->root);

    // TODO: Declaration Phase
    fprintf(stderr, "global ");
    er_symtab_print_all(&bmod->globals);

    er_create_buildfuncs(bmod);

    er_irgen(bmod);

    if (!er_analyze(bmod)) {
        goto end;
    }

    er_codegen(bmod);

    mods = er_xmalloc(2 * sizeof(er_mod_t *));
    mods[0] = bmod->mod;
    mods[1] = NULL;

end:
    if (bmod != NULL) {
        er_buildmod_delete(bmod);
    }

    er_buildctx_destruct(&bctx);

    return mods;
}
