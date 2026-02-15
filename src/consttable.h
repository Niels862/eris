#ifndef ER_CONSTTABLE_H
#define ER_CONSTTABLE_H

#include <stdint.h>

typedef enum {
    ER_CONST_S64,
    ER_CONST_STR,
    ER_CONST_FUNCREF,
} er_consttag_t;

#define ER_CONST_HEADER \
    uint8_t tag /* = er_consttag_t */

typedef struct {
    ER_CONST_HEADER;
} er_const_t;

typedef struct {
    ER_CONST_HEADER;
    int64_t s64;
} er_const_s64_t;

typedef struct {
    ER_CONST_HEADER;
    uint16_t size;
    char str[];
} er_const_str_t;

typedef struct {
    ER_CONST_HEADER;
    uint16_t modidx;
    uint16_t nameidx;
} er_const_funcref_t;

#endif
