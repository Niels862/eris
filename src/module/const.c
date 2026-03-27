#include "module/const.h"
#include <stdio.h>
#include <inttypes.h>

void er_const_print(er_const_t *c) {
    switch (c->tag) {
        case ER_CONST_STR: {
            er_const_str_t *str = (er_const_str_t *)c;
            fprintf(stderr, "STR: \"%.*s\"\n", str->len, str->data);
            break;
        }

        case ER_CONST_MODREF: {
            er_const_modref_t *modref = (er_const_modref_t *)c;
            fprintf(stderr, "MODREF: #%" PRIu16 "\n", modref->name);
            break;
        }

        case ER_CONST_FUNCREF: {
            er_const_funcref_t *funcref = (er_const_funcref_t *)c;
            fprintf(stderr, 
                    "FUNCREF: #%" PRIu16 ":#%" PRIu16 "\n", 
                    funcref->mod, funcref->name);
            break;
        }

        default:
            ER_UNHANDLED_SWITCH_VALUE("%" PRIu16, c->tag);
    }
}
