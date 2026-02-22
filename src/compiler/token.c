#include "token.h"
#include <stddef.h>

#define X(n, l) [ER_TOK_##n] = #n,
static char const * const er_tokkind_names[] = {
    ER_TOKENS(X)
};
#undef X

static size_t const er_n_tokens 
        = sizeof(er_tokkind_names) / sizeof(*er_tokkind_names);

char const *er_tokkind_name(er_tokkind_t kind) {
    if (kind < er_n_tokens) {
        return er_tokkind_names[kind];
    }
    return NULL;
}

void er_tok_print(er_tok_t *tok, FILE *file) {
    fprintf(file, "[%s]\n", er_tokkind_name(tok->kind));
}
