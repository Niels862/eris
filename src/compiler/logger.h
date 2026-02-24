#ifndef ER_COMPILER_LOGGER_H
#define ER_COMPILER_LOGGER_H

#include "compiler/build.h"
#include "util/string.h"

__attribute__((format(printf, 3, 4)))
void er_err(er_buildmod_t *bmod, er_textpos_t pos, char const *msg, ...);

#endif
