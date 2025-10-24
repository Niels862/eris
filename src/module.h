#ifndef ERIS_MODULE_H
#define ERIS_MODULE_H

#include "instruction.h"
#include "ctk/dynamic-array.h"
#include <stddef.h>
#include <stdint.h>
#include <assert.h>

typedef struct {
    uint8_t *code;
    size_t codesize;
    uint32_t *ctable;
    size_t ctablesize;
    uint8_t *cdata;
} eris_module_t;

void eris_module_init(eris_module_t *mod, 
                      uint8_t *code, size_t codesize, 
                      uint32_t *ctable, size_t ctablesize,
                      uint8_t *cdata);

void eris_module_destruct(eris_module_t *mod);

void eris_module_ctable_write(eris_module_t *mod);

static inline void *eris_module_get_const(eris_module_t *mod, 
                                          eris_cindex_t idx) {
    assert(idx < mod->ctablesize);
    return &mod->cdata[mod->ctable[idx]];
}

#endif
