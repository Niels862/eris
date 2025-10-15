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

#define ERIS_TOKENS_KEYWORD(X) \
        X(FUNCTION, "function"), \
        X(RETURN, "return"), \
        X(IF, "if"), \
        X(ELSE, "else"), \
        X(WHILE, "while")

#define ERIS_TOKENS_OPERATOR(X) \
        X(ARROW, "->"), \
        X(PLUS, "+"), \
        X(MINUS, "-"), \
        X(ASTERISK, "*"), \
        X(SLASH, "/"), \
        X(DSLASH, "//"), \
        X(EQUAL, "="), \
        X(DEQUAL, "=="), \
        X(LT, "<"), \
        X(GT, ">"), \
        X(LTEQ, "<="), \
        X(GTEQ, ">="), \
        X(EXCL, "!"), \
        X(EQCLEQ, "!=")

#define ERIS_TOKENS_SEPARATOR(X) \
        X(LBRACE, "{"), \
        X(RBRACE, "}"), \
        X(LPAREN, "("), \
        X(RPAREN, ")"), \
        X(LBRACK, "["), \
        X(RBRACK, "]"), \
        X(SEMICOLON, ";"), \
        X(COLON, ":")

#define ERIS_TOKENS_OTHER(X) \
        X(UNRECOGNIZED, ""), \
        X(EOF, "")

#define ERIS_TOKENS(X) \
        ERIS_TOKENS_NULL(X), \
        ERIS_TOKENS_LITERAL(X), \
        ERIS_TOKENS_KEYWORD(X), \
        ERIS_TOKENS_OPERATOR(X), \
        ERIS_TOKENS_SEPARATOR(X), \
        ERIS_TOKENS_OTHER(X)

#define ERIS_X_EXPAND_ENUM(e, s) ERIS_TOKEN_##e
#define ERIS_X_EXPAND_NAME(e, s) [ERIS_TOKEN_##e] = #e
#define ERIS_X_EXPAND_FIXED(e, s) { s, ERIS_TOKEN_##e }

typedef enum {
    ERIS_TOKENS(ERIS_X_EXPAND_ENUM)
} eris_tokenkind_t;

extern ctk_zstr_t eris_token_names[];

#endif
