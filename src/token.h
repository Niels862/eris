#ifndef ERIS_TOKENKIND_H
#define ERIS_TOKENKIND_H

#include "ctk/string-utils.h"

/* X(name, string) 
 * Expands to ERIS_TOKEN_name, with "name" as string representation.
 */

#define ERIS_TOKENS_NULL(X) \
        X(NULL, "")

#define ERIS_TOKENS_LITERAL(X) \
        X(IDENTIFIER, ""), \
        X(INTLIT, "") \

#define ERIS_TOKENS_OTHER(X) \
        X(UNRECOGNIZED, ""), \
        X(EOF, "")

#define ERIS_TOKENS(X) \
        ERIS_TOKENS_NULL(X), \
        ERIS_TOKENS_LITERAL(X), \
        ERIS_TOKENS_OTHER(X)

#define ERIS_X_EXPAND_ENUM(e, s) ERIS_TOKEN_##e
#define ERIS_X_EXPAND_NAME(e, s) [ERIS_TOKEN_##e] = #e

typedef enum {
    ERIS_TOKENS(ERIS_X_EXPAND_ENUM)
} eris_tokenkind_t;

extern ctk_zstr_t eris_token_names[];

#endif
