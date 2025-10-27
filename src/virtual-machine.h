#ifndef ERIS_VIRTUAL_MACHINE_H
#define ERIS_VIRTUAL_MACHINE_H

#include "module.h"
#include "constant-table.h"

void eris_run(eris_module_t *mod, eris_const_function_t *entry);

#endif
