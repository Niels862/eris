#ifndef ER_COMPILER_DECLARE_H
#define ER_COMPILER_DECLARE_H

#include "compiler/symbol.h"
#include "compiler/build.h"
#include <stdbool.h>

void er_load_builtins(er_buildctx_t *bctx);

bool er_declare(er_buildctx_t *bctx, er_buildmod_t *bmod);

#endif
