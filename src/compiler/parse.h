#ifndef ER_COMPILER_PARSER_H
#define ER_COMPILER_PARSER_H

#include "compiler/build.h"
#include "compiler/token.h"
#include "compiler/ast.h"
#include <stdbool.h>

bool er_parse(er_buildmod_t *bmod);

#endif
