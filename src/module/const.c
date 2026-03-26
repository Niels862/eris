#include "module/const.h"
#include <stdio.h>
#include <inttypes.h>

void er_const_print(er_const_t *c) {
    switch (c->tag) {
        case ER_CONST_STR: {
            er_const_str_t *str = (er_const_str_t *)c;
            fprintf(stderr, "STR: \"%.*s\" (%" PRId16 "B)\n", 
                    str->len, str->data, str->len);
            break;
        }

        case ER_CONST_MODREF: {
            er_const_modref_t *modref = (er_const_modref_t *)c;
            fprintf(stderr, "MODREF: name = [%d]\n", modref->name);
            break;
        }

        default:
            ER_UNHANDLED_SWITCH_VALUE("%d", c->tag);
    }
}
