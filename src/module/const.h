#ifndef ER_MODULE_CONST_H
#define ER_MODULE_CONST_H

#include "util/string.h"
#include "util/error.h"
#include <inttypes.h>
#include <stdint.h>

typedef uint16_t er_constidx_t;

typedef enum {
    ER_CONST_S64,
    ER_CONST_STR,
    ER_CONST_MODREF,
    ER_CONST_FUNCREF,
} er_consttag_t;

#define ER_CONST_HEADER uint8_t tag /* er_consttag_t */

typedef struct {
    ER_CONST_HEADER;
} er_const_t;

typedef struct {
    ER_CONST_HEADER;
    uint16_t len;
    char data[];
} er_const_str_t;

typedef struct {
    ER_CONST_HEADER;
    er_constidx_t name;
} er_const_modref_t;

typedef struct {
    ER_CONST_HEADER;
    er_constidx_t mod;
    er_constidx_t name;
} er_const_funcref_t;

void er_const_print(er_const_t *c);

#endif
