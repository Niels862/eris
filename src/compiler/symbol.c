#include "compiler/symbol.h"
#include "util/list.h"
#include <stdlib.h>

#define ER_SYMTAB_CAP_FROZEN 0

void er_symtab_init(er_symtab_t *syms) {
    ER_LIST_INIT(syms, 4);
}

void er_symtab_destruct(er_symtab_t *syms) {
    if (syms->cap != ER_SYMTAB_CAP_FROZEN) {
        free(syms->data);
    }
}

void er_symtab_freeze(er_symtab_t *syms, er_arena_t *arena) {
    er_arena_realloc(arena, syms->data, 
                     syms->size, syms->size, sizeof(er_sym_t *));
    syms->cap = ER_SYMTAB_CAP_FROZEN;
}
