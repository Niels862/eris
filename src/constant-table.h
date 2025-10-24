#ifndef ERIS_CONSTANT_TABLE_H
#define ERIS_CONSTANT_TABLE_H

#include "instruction.h"
#include <stdint.h>

typedef enum {
    ERIS_CONST_NONE,
    ERIS_CONST_FUNCTION,
} eris_centry_kind_t;

typedef uint8_t  eris_centry_id_t;
typedef uint16_t eris_mod_id_t;

typedef struct {
    eris_centry_id_t kind;
    eris_mod_id_t mod;
    uint32_t codestart;
    uint32_t codesize;
} eris_const_function_t;

extern const size_t eris_centry_size[];
extern const size_t eris_centry_align[];

void eris_const_write(void *vc, uint8_t *code);

void eris_ctable_write(uint32_t *ctable, uint32_t ctablesize, 
                       uint8_t *cdata, uint8_t *code);

#endif
