#include "module/const.h"
#include <stdio.h>
#include <inttypes.h>

void er_const_print(er_const_t *c) {
    switch (c->tag) {
        case ER_CONST_S64:
            fprintf(stderr, "S64: %" PRId64 "\n", c->data.s64);
            break;

        case ER_CONST_STR:
            fprintf(stderr, "STR: \"%.*s\" (%" PRId16 "B)\n", 
                    c->data.str.len, c->data.str.data, c->data.str.len);
            break;

        default:
            ER_UNHANDLED_SWITCH_VALUE("%d", c->tag);
    }
}
