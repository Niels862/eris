#ifndef ER_COMPILER_PARSER_H
#define ER_COMPILER_PARSER_H

#include "compiler/build.h"
#include "compiler/token.h"
#include "compiler/ast.h"

er_astnode_t *er_parse(er_buildmod_t *bmod, er_tok_t *toks);

#endif
