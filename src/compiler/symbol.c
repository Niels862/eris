#include "compiler/symbol.h"
#include "util/list.h"
#include <stdlib.h>
#include <string.h>

#define ER_SYMTAB_CAP_FROZEN 0

void er_sym_print(er_sym_t *sym) {
    fprintf(stderr, "%.*s: ", sym->name.len, sym->name.data);

    switch (sym->kind) {
        case ER_SYM_CLASS: {
            er_symclass_t *Class = &sym->data.Class;
            fprintf(stderr, "[class] valuetype=");
            er_type_print(Class->type);
            fprintf(stderr, "\n");
            break;
        }

        case ER_SYM_FUNC: {
            fprintf(stderr, "[func]\n");
            break;
        }
    }
}

void er_symtab_init(er_symtab_t *syms) {
    ER_LIST_INIT(syms, 4);
    syms->enclosing = NULL;
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

static void er_symtab_print_contents(er_symtab_t *syms) {
    fprintf(stderr, "{\n");

    if (syms->size == 0) {
        fprintf(stderr, "  (empty)\n");
    } else {
        for (size_t i = 0; i < syms->size; i++) {
            fprintf(stderr, "  ");
            er_sym_print(syms->data[i]);
        }
    }

    fprintf(stderr, "}\n");
}

void er_symtab_print(er_symtab_t *syms) {
    fprintf(stderr, "symbol-table ");
    er_symtab_print_contents(syms);
}

void er_symtab_print_all(er_symtab_t *syms) {
    fprintf(stderr, "symbol-table ");

    size_t i = 0;
    while (syms != NULL) {
        fprintf(stderr, "(level %zu) ", i);
        er_symtab_print_contents(syms);

        syms = syms->enclosing;
        i++;
    }
}

er_sym_t *er_symtab_insert(er_symtab_t *syms, er_sym_t *sym) {
    ER_LIST_ADD(syms, sym);
    return NULL;
}

er_sym_t *er_symtab_lookup(er_symtab_t *syms, er_str_t *name) {
    if (syms == NULL) {
        return NULL;
    }

    for (size_t i = 0; i < syms->size; i++) {
        er_sym_t *sym = syms->data[i];

        if (sym->name.len == name->len 
            && strncmp(sym->name.data, name->data, name->len) == 0) {
            return sym;
        }
    }

    return er_symtab_lookup(syms->enclosing, name);
}
