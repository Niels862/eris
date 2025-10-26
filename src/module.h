#ifndef ERIS_MODULE_H
#define ERIS_MODULE_H

#include "instruction.h"
#include "ctk/dynamic-array.h"
#include "ctk/pool.h"
#include <stddef.h>
#include <stdint.h>
#include <assert.h>

typedef struct {
    uint8_t *code;
    size_t codesize;
    void **ctable;
    size_t ctablesize;
    ctk_pool_t cpool;
} eris_module_t;

void eris_module_init(eris_module_t *mod, 
                      uint8_t *code, size_t codesize, 
                      void **ctable, size_t ctablesize,
                      ctk_pool_t *cpool);

void eris_module_destruct(eris_module_t *mod);

void eris_module_ctable_write(eris_module_t *mod);

static inline void *eris_module_get_const(eris_module_t *mod, 
                                          eris_cindex_t idx) {
    assert(idx < mod->ctablesize);
    return mod->ctable[idx];
}

#endif
