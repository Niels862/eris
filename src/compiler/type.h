#ifndef ER_COMPILER_TYPE_H
#define ER_COMPILER_TYPE_H

#include "util/arena.h"
#include <stddef.h>

typedef struct er_sym_t er_sym_t;

typedef struct er_type_t er_type_t;

typedef enum {
    ER_TYPE_FUNC,
    ER_TYPE_CLASS,
} er_typekind_t;

typedef struct {
    er_sym_t *sym;
} er_typeclass_t;

typedef struct {
    er_type_t *rettype;
} er_typefunc_t;

typedef union {
    er_typeclass_t Class;
    er_typefunc_t Func;
} er_typedata_t;

struct er_type_t {
    er_typekind_t kind;
    er_typedata_t data;
};

typedef struct {
    er_arena_t *arena;

    struct {
        er_type_t **data;
        size_t size;
        size_t cap;
    } types;
} er_typefactory_t;

void er_type_print(er_type_t *type);

void er_typefactory_init(er_typefactory_t *tf);

void er_typefactory_destruct(er_typefactory_t *tf);

er_type_t *er_make_classtype(er_typefactory_t *tf, er_sym_t *sym);

er_type_t *er_make_functype(er_typefactory_t *tf, er_type_t *rettype);

#endif
