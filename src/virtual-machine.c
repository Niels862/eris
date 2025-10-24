#include "virtual-machine.h"

void eris_run(eris_module_t *mod) {
    void *cfunc = eris_module_get_const(mod, 0);

    (void)cfunc;
}
