#include "compiler/type.h"
#include "compiler/symbol.h"
#include <stdio.h>
#include <string.h>

void er_type_print(er_type_t *type) {
    switch (type->kind) {
        case ER_TYPE_CLASS: {
            er_typeclass_t *Class = &type->data.Class;
            er_sym_t *sym = Class->sym;
            fprintf(stderr, "%.*s", sym->name.len, sym->name.data);
            break;
        }

        case ER_TYPE_FUNC: {
            er_typefunc_t *Func = &type->data.Func;
            fprintf(stderr, "() -> ");
            er_type_print(Func->rettype);
            break;
        }
    }
}

void er_typefactory_init(er_typefactory_t *tf) {
    tf->arena = er_arena_new(1024);
}

void er_typefactory_destruct(er_typefactory_t *tf) {
    er_arena_delete(tf->arena);
}

static er_typedata_t const er_dummy_typedata;

static er_type_t *er_type_alloc(er_arena_t *arena, er_typekind_t kind, 
                                size_t datasize) {
    size_t size = offsetof(er_type_t, data) + datasize;
    er_type_t *type = er_arena_alloc(arena, size);

    type->kind = kind;
    memset(&type->data, 0, datasize);

    return type;
}

#define ER_TYPE_ALLOC(arena, kind, data) \
        er_type_alloc(arena, kind, sizeof(er_dummy_typedata.data))

er_type_t *er_make_classtype(er_typefactory_t *tf, er_sym_t *sym) {
    er_type_t *type = ER_TYPE_ALLOC(tf->arena, ER_TYPE_CLASS, Class);
    type->data.Class.sym = sym;

    return type;
}
