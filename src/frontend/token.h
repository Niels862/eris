#ifndef ERIS_TOKENKIND_H
#define ERIS_TOKENKIND_H

#include "ctk/token.h"
#include "ctk/string-utils.h"

/* X(name, string) 
 * Expands to ERIS_TOKEN_name, with "name" as string representation.
 */

#define ERIS_TOKENS_NULL(X) \
        X(NULL, "(null)", "")

#define ERIS_TOKENS_LITERAL(X) \
        X(IDENTIFIER, "identifier", "") \
        X(NUMBER, "number", "") \

#define ERIS_TOKENS_KEYWORD(X) \
        X(FUNCTION, "'function'", "function") \
        X(RETURN, "'return'", "return") \
        X(IF, "'if'", "if") \
        X(ELSE, "'else'", "else") \
        X(WHILE, "'while'", "while")

#define ERIS_TOKENS_OPERATOR(X) \
        X(ARROW, "'->'", "->") \
        X(PLUS, "'+'", "+") \
        X(MINUS, "'-'", "-") \
        X(ASTERISK, "'*'", "*") \
        X(SLASH, "'/'", "/") \
        X(DSLASH, "'//'", "//") \
        X(EQUAL, "'='", "=") \
        X(DEQUAL, "'=='", "==") \
        X(LT, "'<'", "<") \
        X(GT, "'>'", ">") \
        X(LTEQ, "'<='", "<=") \
        X(GTEQ, "'>='", ">=") \
        X(EXCL, "'!'", "!") \
        X(EXCLEQ, "'!='", "!=")

#define ERIS_TOKENS_SEPARATOR(X) \
        X(LBRACE, "'{'", "{") \
        X(RBRACE, "'}'", "}") \
        X(LPAREN, "'('", "(") \
        X(RPAREN, "')'", ")") \
        X(LBRACK, "'['", "[") \
        X(RBRACK, "']'", "]") \
        X(SEMICOLON, "';'", ";") \
        X(COLON, "':'", ":")

#define ERIS_TOKENS_OTHER(X) \
        X(UNRECOGNIZED, "unrecognized token", "") \
        X(EOF, "end of input", "")

#define ERIS_TOKENS(X) \
        ERIS_TOKENS_NULL(X) \
        ERIS_TOKENS_LITERAL(X) \
        ERIS_TOKENS_KEYWORD(X) \
        ERIS_TOKENS_OPERATOR(X) \
        ERIS_TOKENS_SEPARATOR(X) \
        ERIS_TOKENS_OTHER(X)

#define ERIS_TOKEN_X_EXPAND_ENUM(e, n, s) ERIS_TOKEN_##e,
#define ERIS_TOKEN_X_EXPAND_NAME(e, n, s) [ERIS_TOKEN_##e] = #e,
#define ERIS_TOKEN_X_EXPAND_INFO_NAME(e, n, s) [ERIS_TOKEN_##e] = n,
#define ERIS_TOKEN_X_EXPAND_FIXED(e, n, s) { s, ERIS_TOKEN_##e },

typedef enum {
    ERIS_TOKENS(ERIS_TOKEN_X_EXPAND_ENUM)
} eris_tokenkind_t;

extern ctk_zstr_t eris_token_names[];
extern ctk_zstr_t eris_token_info_names[];

#endif
