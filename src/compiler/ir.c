#include "ir.h"
#include <stdio.h>
#include <inttypes.h>
#include <assert.h>

#define X(n, k) [ER_IR_##n] = #n,
static char const * const er_irtag_names[] = {
    ER_IRTAGS(X)
};
#undef X

#define X(n, k) [ER_IR_##n] = ER_IRDATA_##k,
static er_irdatakind_t const er_irtag_datakinds[] = {
    ER_IRTAGS(X)
};
#undef X

static char const *er_ir_name(er_irtag_t tag) {
    if (tag < sizeof(er_irtag_names) / sizeof(*er_irtag_names)) {
        return er_irtag_names[tag];
    } else {
        return NULL;
    }
}

static er_irdatakind_t er_ir_kind(er_irtag_t tag) {
    assert(tag < sizeof(er_irtag_datakinds) / sizeof(*er_irtag_datakinds));
    return er_irtag_datakinds[tag];
}

void er_irnode_print(er_irnode_t *node) {
    er_irdatakind_t kind = er_ir_kind(node->tag);
    
    int len = fprintf(stderr, "%" PRId16 ":%" PRId16, 
                      node->pos.line, node->pos.col);

    for (int i = 0; i < 8 - len; i++) {
        fputc(' ', stderr);
    }

    fprintf(stderr, "%s ", er_ir_name(node->tag));

    switch (kind) {
        case ER_IRDATA_NONE:
            fprintf(stderr, "\n");
            break;

        case ER_IRDATA_S64:
            fprintf(stderr, "% " PRId64 "\n", node->data.s64);
            break;
    }
}
