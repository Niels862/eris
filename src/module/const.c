#include "module/const.h"
#include <stdio.h>
#include <inttypes.h>

void er_const_print(er_const_t *c) {
    switch (c->tag) {
        case ER_CONST_S64:
            fprintf(stderr, "S64: %" PRId64 "\n", c->data.s64);
            break;

        default:
            ER_FATAL("Unhandled switch value: %d\n", c->tag);
    }
}
