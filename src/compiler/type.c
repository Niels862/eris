#include "compiler/type.h"
#include "compiler/symbol.h"
#include "util/list.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
            fprintf(stderr, "()->");
            er_type_print(Func->rettype);
            break;
        }
    }
}

void er_typefactory_init(er_typefactory_t *tf) {
    tf->arena = er_arena_new(1024);
    ER_LIST_INIT(&tf->types, 16);
}

void er_typefactory_destruct(er_typefactory_t *tf) {
    er_arena_delete(tf->arena);
    free(tf->types.data);
}

static size_t er_type_sizeof(er_type_t *t) {
    size_t headersize = offsetof(er_type_t, data);

    switch (t->kind) {
        case ER_TYPE_CLASS: {
            return headersize + sizeof(er_typeclass_t);
        }

        case ER_TYPE_FUNC: {
            return headersize + sizeof(er_typefunc_t);
        }
    }

    ER_UNHANDLED_SWITCH_VALUE("%d", t->kind);
}

static bool er_type_equal(er_type_t *t1, er_type_t *t2) {
    if (t1->kind != t2->kind) {
        return false;
    }

    switch (t1->kind) {
        case ER_TYPE_CLASS: {
            return t1->data.Class.sym == t2->data.Class.sym;
        }

        case ER_TYPE_FUNC: {
            return t1->data.Func.rettype == t2->data.Func.rettype;
        }
    }

    ER_UNHANDLED_SWITCH_VALUE("%d", t1->kind);
}

static er_type_t *er_make_type(er_typefactory_t *tf, er_type_t *t) {
    for (size_t i = 0; i < tf->types.size; i++) {
        er_type_t *t2 = tf->types.data[i];

        if (er_type_equal(t2, t)) {
            return t2;
        }
    }

    size_t size = er_type_sizeof(t);

    er_type_t *t2 = er_arena_alloc(tf->arena, size);
    memcpy(t2, t, size);
    ER_LIST_ADD(&tf->types, t2);

    return t2;
}

er_type_t *er_make_classtype(er_typefactory_t *tf, er_sym_t *sym) {
    er_type_t type = {
        .kind = ER_TYPE_CLASS,
        .data.Class = {
            .sym = sym
        }
    };

    return er_make_type(tf, &type);
}

er_type_t *er_make_functype(er_typefactory_t *tf, er_type_t *rettype) {
    er_type_t type = {
        .kind = ER_TYPE_FUNC,
        .data.Func = {
            .rettype = rettype
        }
    };

    return er_make_type(tf, &type);
}
