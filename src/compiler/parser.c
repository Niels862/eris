#include "compiler/parser.h"
#include "util/error.h"
#include <stddef.h>

typedef struct {
    er_buildmod_t *bmod;
    er_tok_t *toks;
    er_tok_t *curr;
} er_parsectx_t;

er_astnode_t *er_parse(er_buildmod_t *bmod, er_tok_t *toks) {
    er_parsectx_t ctx = {
        .bmod = bmod,
        .toks = toks,
        .curr = toks,
    };

    ER_UNUSED(ctx);

    return NULL;
}
