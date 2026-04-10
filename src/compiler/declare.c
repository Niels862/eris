#include "compiler/declare.h"
#include "util/list.h"
#include "util/error.h"
#include <string.h>
#include <assert.h>

static er_symdata_t const er_dummy_symdata;

static er_textpos_t const er_textpos_empty = { 0, 0 };

er_sym_t *er_sym_alloc(er_arena_t *arena, er_symkind_t kind, er_str_t *name, 
                       er_textpos_t pos, size_t datasize) {
    size_t size = offsetof(er_sym_t, data) + datasize;
    er_sym_t *sym = er_arena_alloc(arena, size);

    sym->kind = kind;
    sym->name = *name;
    sym->declpos = pos;
    memset(&sym->data, 0, datasize);

    return sym;
}

#define ER_SYM_ALLOC(arena, kind, name, pos, data) \
        er_sym_alloc(arena, kind, name, pos, sizeof(er_dummy_symdata.data))

static er_sym_t *er_make_class(er_arena_t *arena, 
                               er_str_t *name, er_textpos_t pos) {    
    return ER_SYM_ALLOC(arena, ER_SYM_CLASS, name, pos, Class);
}

static er_sym_t *er_make_function(er_arena_t *arena, 
                                  er_str_t *name, er_textpos_t pos) {
    return ER_SYM_ALLOC(arena, ER_SYM_FUNC, name, pos, Func);
}

static er_sym_t *er_insert_builtin_classtype(er_buildctx_t *bctx, char *name, 
                                             er_type_t **type) {
    er_arena_t *arena = bctx->arenas.persistent;
    
    er_str_t str;
    er_str_from_cstr(&str, name);

    er_sym_t *sym = er_make_class(arena, &str, er_textpos_empty);
    er_sym_t *prev = er_symtab_insert(&bctx->builtins, sym);
    assert(prev == NULL);

    *type = er_make_classtype(&bctx->tf, sym);
    sym->data.Class.type = *type;

    return sym;
}

void er_load_builtins(er_buildctx_t *bctx) {
    bctx->sym.Int = er_insert_builtin_classtype(bctx, "int", &bctx->Int);
    bctx->sym.Bool = er_insert_builtin_classtype(bctx, "bool", &bctx->Bool);
}

typedef struct {
    er_arena_t *arena;
    er_typefactory_t *tf;

    struct {
        er_buildfunc_t *data;
        size_t size;
        size_t cap;
    } bfuncs;
} er_declctx_t;

static er_type_t *er_type_from_anno(er_astnode_t *anno, er_symtab_t *syms) {
    switch (anno->kind) {
        case ER_AST_IDENT: {
            er_sym_t *sym = er_symtab_lookup(syms, &anno->data.Ident.name);

            if (sym->kind == ER_SYM_CLASS) {
                return sym->data.Class.type;
            }

            ER_FATAL("TODO");
        }

        default:
            ER_UNHANDLED_SWITCH_VALUE("%d", anno->kind);
    }

    return NULL;
}

static bool er_declare_walk(er_declctx_t *dctx, er_astnode_t *n, 
                            er_symtab_t *syms) {
    bool s = true;

    switch (n->kind) {
        case ER_AST_MOD: {
            er_astmod_t *Mod = &n->data.Mod;

            for (size_t i = 0; i < Mod->n_funcs; i++) {
                s &= er_declare_walk(dctx, Mod->funcs[i], syms);
            }
            break;
        }

        case ER_AST_FUNC: {
            er_astfunc_t *Func = &n->data.Func;

            er_type_t *rettype = er_type_from_anno(Func->ret_anno, syms);

            er_sym_t *sym = er_make_function(dctx->arena, &Func->name, n->pos);
            sym->data.Func.type = er_make_functype(dctx->tf, rettype);

            er_symtab_insert(syms, sym);

            er_buildfunc_t *bfunc = ER_LIST_EMPLACE(&dctx->bfuncs);
            er_buildfunc_init(bfunc, n, sym);

            break;
        }

        default: 
            ER_UNHANDLED_SWITCH_VALUE("%d", n->kind);
    }

    return s;
}

bool er_declare(er_buildctx_t *bctx, er_buildmod_t *bmod) {
    er_declctx_t dctx = {
        .tf     = &bctx->tf,
        .arena  = bmod->arenas.persistent,
    };
    ER_LIST_INIT(&dctx.bfuncs, 8);

    bool s = er_declare_walk(&dctx, bmod->root, &bmod->globals);

    bmod->bfuncs = dctx.bfuncs.data;
    bmod->n_bfuncs = dctx.bfuncs.size;

    return s;
}
