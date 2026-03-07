#ifndef ER_COMPILER_LOWER_H
#define ER_COMPILER_LOWER_H

#include "compiler/build.h"
#include "compiler/ast.h"

// FIXME: FUNC to be replaced by FUNC_INFO object
void er_lower(er_buildmod_t *bmod, er_astnode_t *func);

#endif
