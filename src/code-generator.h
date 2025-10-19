#ifndef ERIS_CODE_GENERATOR_H
#define ERIS_CODE_GENERATOR_H

#include "ctk/dynamic-array.h"

typedef struct {
    ctk_dynarr_t code;
} eris_codegen_t;

void eris_codegen_init(eris_codegen_t *gen);

void eris_codegen_destruct(eris_codegen_t *gen);

#endif
