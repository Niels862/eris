#ifndef ER_COMPILER_LEXER_H
#define ER_COMPILER_LEXER_H

#include "compiler/token.h"
#include "compiler/build.h"
#include <stddef.h>

er_tok_t *er_lex(er_buildmod_t *bmod);

#endif
