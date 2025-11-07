#include "runtime/loader.h"
#include "ctk/map.h"
#include "ctk/allocator.h"
#include "stdio.h"
#include "string.h"
#include "stdbool.h"

typedef struct {
    eris_module_t *mod;
    ctk_linmap_t links;
} eris_loader_t;

ctk_hash_t eris_conststr_hash(void *p) {
    (void)p;
    return 0;
}

bool eris_conststr_is_equal(void *p1, void *p2) {
    eris_const_str_t *s1 = p1;
    eris_const_str_t *s2 = p2;
    return s1->size == s2->size && strncmp(s1->data, s2->data, s1->size) == 0;
}

void eris_loader_init(eris_loader_t *ld, eris_module_t *mod) {
    static ctk_map_definition_t const conststr_map_def = {
        .hash       = &eris_conststr_hash,
        .is_equal   = &eris_conststr_is_equal,
    };

    ld->mod = mod;
    ctk_linmap_init(&ld->links, &conststr_map_def);
}

void eris_loader_destruct(eris_loader_t *ld) {
    ctk_linmap_destruct(&ld->links);
}

static inline void eris_insert_link(eris_module_t *mod, ctk_linmap_t *links, 
                                    eris_const_str_t *cs, void *c) {
    (void)mod; // TODO: use this to throw (fatal?) error
    void *prev = ctk_linmap_insert(links, cs, c);
    assert(prev == NULL);
}

static inline void eris_load_module_links(eris_module_t *mod, 
                                          ctk_linmap_t *links) {
    for (size_t i = 0; i < mod->ctablesize; i++) {
        void *vc = mod->ctable[i];

        switch (*(uint8_t *)vc) {
            case ERIS_CONST_FUNCTION: {
                eris_const_function_t *c = vc;
                eris_insert_link(mod, links, mod->ctable[c->name], c);
                break;
            }
        }
    }
}

void eris_loader_load_links(eris_loader_t *ld) {
    eris_load_module_links(ld->mod, &ld->links);
}

void *eris_loader_resolve(eris_loader_t *ld, eris_const_str_t *cs) {
    return ctk_linmap_lookup(&ld->links, cs);
}

void *eris_loader_resolve_string(eris_loader_t *ld, char *s) {
    static uint8_t buf[sizeof(eris_const_str_t) + 256];

    size_t size = strlen(s);
    assert(size < 256);
    
    eris_const_str_t *cs = (eris_const_str_t *)buf;
    cs->kind = ERIS_CONST_STR;
    cs->size = size;
    memcpy(cs->data, s, size);

    return eris_loader_resolve(ld, cs);
}

eris_const_function_t *eris_load(eris_module_t *mod) {
    eris_loader_t ld;
    eris_loader_init(&ld, mod);

    eris_loader_load_links(&ld);

    void *p = eris_loader_resolve_string(&ld, "main");
    fprintf(stderr, "\nentry => ");

    if (p == NULL) {
        fprintf(stderr, "(unresolved)\n");
    } else {
        eris_const_write(p, mod->code, mod->ctable);
    }
    fprintf(stderr, "\n");

    eris_loader_destruct(&ld);

    return p;
}
