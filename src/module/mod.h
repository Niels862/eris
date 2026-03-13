#ifndef ERIS_MODULE_MOD_H
#define ERIS_MODULE_MOD_H

#include "module/const.h"
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint16_t me;
    uint16_t modid;
    uint8_t *code;
    size_t code_size;
} er_func_t;

typedef struct {
    er_const_t *consts;
    size_t n_consts;

    er_func_t *funcs;
    size_t n_funcs;

    uint16_t me;
} er_mod_t;

void er_mod_print(er_mod_t *mod);

#endif
