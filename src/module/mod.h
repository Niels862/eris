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
    uint16_t me;

    er_const_t **consts;
    size_t n_consts;

    er_func_t **funcs;
    size_t n_funcs;
} er_mod_t;

er_func_t *er_func_new(uint16_t me, uint16_t modid, 
                       uint8_t *code, size_t code_size);

void er_func_delete(er_func_t *func);

er_mod_t *er_mod_new(uint16_t me, er_const_t **consts, size_t n_consts, 
                     er_func_t **funcs, size_t n_funcs);

void er_mod_delete(er_mod_t *mod);

void er_mod_print(er_mod_t *mod);

#endif
