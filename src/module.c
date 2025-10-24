#include "module.h"
#include "constant-table.h"
#include "ctk/allocator.h"

void eris_module_init(eris_module_t *mod, 
                      uint8_t *code, size_t codesize, 
                      uint32_t *ctable, size_t ctablesize,
                      uint8_t *cdata) {
    mod->code = code;
    mod->codesize = codesize;
    mod->ctable = ctable;
    mod->ctablesize = ctablesize;
    mod->cdata = cdata;
}

void eris_module_destruct(eris_module_t *mod) {
    ctk_free(&mod->code);
    ctk_free(&mod->ctable);
    ctk_free(&mod->cdata);
}

void eris_module_ctable_write(eris_module_t *mod) {
    eris_ctable_write(mod->ctable, mod->ctablesize, mod->cdata, mod->code);
}
