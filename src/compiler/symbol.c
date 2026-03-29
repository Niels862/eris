#include "compiler/symbol.h"
#include "util/list.h"
#include <stdlib.h>

#define ER_SYMTAB_CAP_FROZEN 0

void er_sym_print(er_sym_t *sym) {
    fprintf(stderr, "%.*s: ", sym->name.len, sym->name.data);

    switch (sym->kind) {
        case ER_SYM_CLASS: {
            fprintf(stderr, "class\n");
            break;
        }
    }
}

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

void er_symtab_print(er_symtab_t *syms) {
    fprintf(stderr, "symbol-table {\n");

    for (size_t i = 0; i < syms->size; i++) {
        fprintf(stderr, "  ");
        er_sym_print(syms->data[i]);
    }

    fprintf(stderr, "}\n");
}

er_sym_t *er_symtab_insert(er_symtab_t *syms, er_sym_t *sym) {
    // FIXME: Find previous definition if exists

    ER_LIST_ADD(syms, sym);

    return NULL;
}
