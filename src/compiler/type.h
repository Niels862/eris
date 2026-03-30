#ifndef ER_COMPILER_TYPE_H
#define ER_COMPILER_TYPE_H

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

void er_type_print(er_type_t *type);

#endif
