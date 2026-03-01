#include "util/string.h"
#include <util/alloc.h>
#include <string.h>

char *er_strdup(char const *s) {
    int len = strlen(s);
    char *s2 = er_xmalloc(len + 1);
    memcpy(s2, s, len + 1);
    return s2;
}
