#include "module.h"
#include "ctk/allocator.h"

void eris_module_init(eris_module_t *mod, uint8_t *code, size_t codesize) {
    mod->code = code;
    mod->codesize = codesize;
}

void eris_module_destruct(eris_module_t *mod) {
    ctk_xfree(mod->code);
}
