#ifndef ERIS_CODE_GENERATOR_H
#define ERIS_CODE_GENERATOR_H

#include "ctk/dynamic-array.h"

typedef struct eris_codegen_t eris_codegen_t;

typedef void(*eris_codegen_visit_t)(eris_codegen_t *gen, void *node);

struct eris_codegen_t {
    ctk_dynarr_t code;
    eris_codegen_visit_t *visitors;
};

void eris_codegen_init(eris_codegen_t *gen, eris_codegen_visit_t *visitors);

void eris_codegen_destruct(eris_codegen_t *gen);

void eris_codegen_dispatch(eris_codegen_t *gen, void *node);

#endif
