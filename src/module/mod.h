#ifndef ERIS_MODULE_MOD_H
#define ERIS_MODULE_MOD_H

#include "module/const.h"
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t *code;
    uint16_t modid;
} er_func_t;

typedef struct {
    er_const_t *consttab;
    void **reftab;
    size_t nconsts;

    er_func_t *functab;
    size_t nfuncs;
} er_module_t;

#endif
