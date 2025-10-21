#ifndef ERIS_MODULE_H
#define ERIS_MODULE_H

#include "ctk/dynamic-array.h"
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint8_t *code;
    size_t codesize;
} eris_module_t;

#endif
