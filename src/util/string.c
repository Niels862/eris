#include "util/string.h"
#include <util/alloc.h>
#include <string.h>

void er_str_from_cstr(er_str_t *s, char *cs) {
    s->data = cs;
    s->len = strlen(cs);
}

char *er_strdup(char const *s) {
    int len = strlen(s);
    char *s2 = er_xmalloc(len + 1);
    memcpy(s2, s, len + 1);
    return s2;
}
