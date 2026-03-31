#include "compiler/declare.h"
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

static er_sym_t *er_insert_builtin_classtype(er_buildctx_t *bctx, char *name) {
    er_arena_t *arena = bctx->arenas.persistent;
    
    er_str_t str;
    er_str_from_cstr(&str, name);

    er_sym_t *sym = er_make_class(arena, &str, er_textpos_empty);
    er_sym_t *prev = er_symtab_insert(&bctx->builtins, sym);
    assert(prev == NULL);

    er_type_t *type = er_make_classtype(&bctx->tf, sym);
    sym->data.Class.type = type;

    return sym;
}

void er_load_builtins(er_buildctx_t *bctx) {
    bctx->sym.Int = er_insert_builtin_classtype(bctx, "int");
}

void er_declare(er_buildmod_t *bmod) {
    ER_UNUSED(bmod);
}
