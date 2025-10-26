#ifndef ERIS_CONSTANT_TABLE_H
#define ERIS_CONSTANT_TABLE_H

#include "instruction.h"
#include <stdint.h>

typedef enum {
    ERIS_CONST_NONE,
    ERIS_CONST_FUNCTION,
    ERIS_CONST_STR,
} eris_centry_kind_t;

typedef uint8_t  eris_centry_id_t;
typedef uint16_t eris_mod_id_t;

typedef struct {
    eris_centry_id_t kind;
    eris_mod_id_t mod;
    eris_cindex_t name;
    uint32_t codestart;
    uint32_t codesize;
} eris_const_function_t;

typedef struct {
    eris_centry_id_t kind;
    uint16_t size;
    char data[];
} eris_const_str_t;

extern const size_t eris_centry_size[];
extern const size_t eris_centry_align[];

void eris_const_write(void *vc, uint8_t *code, void **ctable);

void eris_ctable_write(void **ctable, uint32_t ctablesize, uint8_t *code);

#endif
