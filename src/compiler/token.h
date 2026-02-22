#ifndef ER_COMPILER_TOKEN_H
#define ER_COMPILER_TOKEN_H

#include <stdio.h>

#define ER_KEYWORD_TOKENS(X) \
        X(RETURN, "return")

#define ER_SPECIAL_TOKENS(X) \
        X(SEMICOLON, ";")

#define ER_OTHER_TOKENS(X) \
        X(NUMBER, "") \
        X(ENDOFINPUT, "")

#define ER_TOKENS(X) \
        X(NONE, "") \
        ER_KEYWORD_TOKENS(X) \
        ER_SPECIAL_TOKENS(X) \
        ER_OTHER_TOKENS(X)

#define X(n, l) ER_TOK_##n,
typedef enum {
    ER_TOKENS(X)
} er_tokkind_t;
#undef X

typedef struct {
    er_tokkind_t kind;
} er_tok_t;

char const *er_tokkind_name(er_tokkind_t kind);

void er_tok_print(er_tok_t *tok, FILE *file);

#endif
