#ifndef ER_COMPILER_LEXER_H
#define ER_COMPILER_LEXER_H

#include "compiler/token.h"
#include <stddef.h>

er_tok_t *er_lex(char const *filename, char const *text, size_t size);

#endif
