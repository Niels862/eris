#ifndef ERIS_SYMBOL_TABLE_H
#define ERIS_SYMBOL_TABLE_H

#include "frontend/symbol.h"
#include "ctk/map.h"
#include "ctk/list.h"
#include "ctk/string-span.h"

typedef struct eris_symtable_t eris_symtable_t;

struct eris_symtable_t {
    ctk_linmap_t map;
    eris_symtable_t *enclosing;
};

typedef struct {
    ctk_list_t list; /* of eris_symtable_t */
    size_t idx;
    eris_symtable_t *syms;
} eris_scopelist_t;

void eris_symtable_init(eris_symtable_t *syms);

void eris_symtable_destruct(eris_symtable_t *syms);

void eris_symtable_write(eris_symtable_t *syms);

bool eris_symtable_declare(eris_symtable_t *syms, eris_sym_t *sym);

eris_sym_t *eris_symtable_lookup(eris_symtable_t *syms, ctk_span_t *id);

void eris_scopelist_init(eris_scopelist_t *scopes);

void eris_scopelist_destruct(eris_scopelist_t *scopes);

void eris_scopelist_write(eris_scopelist_t *scopes);

eris_symtable_t *eris_scopelist_push(eris_scopelist_t *scopes);

void eris_scopelist_pop(eris_scopelist_t *scopes);

#endif
