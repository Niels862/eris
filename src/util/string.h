#ifndef ER_COMPILER_STRING_H
#define ER_COMPILER_STRING_H

#include <stdint.h>

typedef struct {
    char *data;
    int len;
} er_str_t;

typedef struct {
    uint16_t line;
    uint16_t col;
} er_textpos_t;

void er_str_from_cstr(er_str_t *s, char *cs);

char *er_strdup(char const *s);

#endif
