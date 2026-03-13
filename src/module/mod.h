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
    er_const_t *consts;
    void **refs;
    size_t n_consts;

    er_func_t *funcs;
    size_t n_funcs;
} er_mod_t;

#endif
