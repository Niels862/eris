#include "module/mod.h"
#include "module/instr.h"
#include <stddef.h>

static void er_func_print(er_func_t *func, er_mod_t *mod) {
    ER_UNUSED(mod);

    fprintf(stderr, "  {\n");

    size_t at = 0;
    while (at < func->code_size) {
        er_instr_print_with_address(at, func->code);
    }
}

void er_mod_print(er_mod_t *mod) {
    fprintf(stderr, "CONSTS {\n");
    
    for (size_t i = 0; i < mod->n_consts; i++) {
        fprintf(stderr, "  %zu\n", i);
    }

    fprintf(stderr, "}\n");
    fprintf(stderr, "FUNCS {\n");

    for (size_t i = 0; i < mod->n_funcs; i++) {
        er_func_print(&mod->funcs[i], mod);
    }

    fprintf(stderr, "}\n");
}
