#ifndef ERIS_SYMBOL_TABLE_H
#define ERIS_SYMBOL_TABLE_H

#include "ctk/map.h"

typedef struct eris_symtable_t eris_symtable_t;

struct eris_symtable_t {
    ctk_linmap_t map;
    eris_symtable_t *enclosing;
};

#endif
