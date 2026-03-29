#ifndef ER_COMPILER_SYMBOL_H
#define ER_COMPILER_SYMBOL_H

#include "util/arena.h"
#include "util/string.h"
#include "util/error.h"
#include <stdbool.h>

#define ER_SYMS(X) \
    X(CLASS)

#define X(n) ER_SYM_##n,
typedef enum {
    ER_SYMS(X)
} er_symkind_t;
#undef X

typedef struct {
    ER_EMPTY;
} er_symclass_t;

typedef struct {
    er_symclass_t Class;
} er_symdata_t;

typedef struct {
    er_symkind_t kind;
    er_str_t name;
    er_textpos_t declpos;
    er_symdata_t data;
} er_sym_t;

typedef struct er_symtab_t er_symtab_t;

struct er_symtab_t {
    er_symtab_t *enclosing;

    er_sym_t **data;
    size_t size;
    size_t cap;
};

void er_sym_print(er_sym_t *sym);

void er_symtab_init(er_symtab_t *syms);

void er_symtab_destruct(er_symtab_t *syms);

void er_symtab_freeze(er_symtab_t *syms, er_arena_t *arena);

void er_symtab_print(er_symtab_t *syms);

er_sym_t *er_symtab_insert(er_symtab_t *syms, er_sym_t *sym);

#endif
