#ifndef ER_MODULE_CONST_H
#define ER_MODULE_CONST_H

#include "util/string.h"
#include "util/error.h"
#include <stdint.h>

typedef enum {
    ER_CONST_S64,
    ER_CONST_STR,
    ER_CONST_MODREF,
    ER_CONST_FUNCREF,
} er_consttag_t;

typedef union {
    int64_t s64;
    er_str_t str;
} er_constdata_t;

typedef struct {
    er_consttag_t tag;
    er_constdata_t data;
} er_const_t;

void er_const_print(er_const_t *c);

#endif
