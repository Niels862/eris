#include "loader.h"
#include "ctk/map.h"
#include "ctk/allocator.h"

typedef struct {
    eris_module_t *mod;
    ctk_linmap_t links;
} eris_loader_t;

void eris_loader_init(eris_loader_t *ld, eris_module_t *mod) {
    ld->mod = mod;
    ctk_linmap_init(&ld->links, &ctk_strmap_def);
}

void eris_loader_destruct(eris_loader_t *ld) {
    while (ctk_linmap_size(&ld->links) > 0) {
        ctk_map_entry_t entry;
        ctk_linmap_pop(&ld->links, &entry);

        ctk_xfree(&entry.key);
    }
}

void eris_loader_load_links(eris_loader_t *ld) {
    (void)ld;
}

void eris_load(eris_module_t *mod) {
    eris_loader_t ld;
    eris_loader_init(&ld, mod);

    eris_loader_destruct(&ld);
}
