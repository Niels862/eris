#ifndef ERIS_CODE_GENERATOR_H
#define ERIS_CODE_GENERATOR_H

#include "frontend/ast.h"
#include "runtime/module.h"
#include "ctk/dynamic-array.h"

void eris_codegen(eris_node_source_t *root, eris_module_t *mod);

#endif
