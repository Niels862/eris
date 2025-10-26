#include "module.h"
#include "constant-table.h"
#include "ctk/allocator.h"

void eris_module_init_empty(eris_module_t *mod) {
    mod->code = NULL;
    mod->codesize = 0;
    mod->ctable = NULL;
    mod->ctablesize = 0;

    // TODO: add empty pool constructor?
    mod->cpool.block = NULL;
    mod->cpool.blocksize = 0;
}

void eris_module_init(eris_module_t *mod, 
                      uint8_t *code, size_t codesize, 
                      void **ctable, size_t ctablesize,
                      ctk_pool_t *cpool) {
    mod->code = code;
    mod->codesize = codesize;
    mod->ctable = ctable;
    mod->ctablesize = ctablesize;
    mod->cpool = *cpool;
    cpool->block = NULL; // TODO: replace by pool_move when available
}

void eris_module_destruct(eris_module_t *mod) {
    ctk_pool_destruct(&mod->cpool);
    ctk_free(&mod->ctable);
    ctk_free(&mod->code);
}

void eris_module_ctable_write(eris_module_t *mod) {
    eris_ctable_write(mod->ctable, mod->ctablesize, mod->code);
}
