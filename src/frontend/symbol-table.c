#include "frontend/symbol-table.h"
#include "ctk/allocator.h"
#include <string.h>
#include <assert.h>

static bool eris_strspan_is_equal(void *p1, void *p2) {
    ctk_strspan_t *span1 = p1;
    ctk_strspan_t *span2 = p2;

    if (span1 == span2) {
        return true;
    }

    if (span1->end - span1->start != span2->end - span2->start) {
        return false;
    }

    return strncmp(span1->start, span2->start, span1->end - span1->start);
}

void eris_symtable_init(eris_symtable_t *syms) {
    static ctk_map_definition_t def = {
        .hash = NULL,
        .is_equal = eris_strspan_is_equal,
    };

    ctk_linmap_init(&syms->map, &def);
}

void eris_symtable_destruct(eris_symtable_t *syms) {
    ctk_linmap_destruct(&syms->map);
}

void eris_scopelist_init(eris_scopelist_t *scopes) {
    ctk_list_init(&scopes->list, 8);
    scopes->idx = 0;
    scopes->syms = NULL;
}

void eris_scopelist_destruct(eris_scopelist_t *scopes) {
    for (size_t i = 0; i < scopes->list.size; i++) {
        (void)i; // TODO: free each list
    }
}

eris_symtable_t *eris_scopelist_push(eris_scopelist_t *scopes) {
    eris_symtable_t *syms = ctk_xmalloc(sizeof(eris_symtable_t));
    eris_symtable_init(syms);
    syms->enclosing = scopes->syms;
    scopes->syms = syms;

    ctk_list_add(&scopes->list, syms);
    
    return syms;
}

void eris_scopelist_pop(eris_scopelist_t *scopes) {
    assert(scopes->syms != NULL);
    scopes->syms = scopes->syms->enclosing;
}
