#ifndef ER_COMPILER_LEXER_H
#define ER_COMPILER_LEXER_H

#include "compiler/token.h"
#include "compiler/build.h"
#include <stddef.h>
#include <stdbool.h>

bool er_lex(er_buildmod_t *bmod);

#endif
