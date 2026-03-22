#include "module/mod.h"
#include "module/instr.h"
#include "util/alloc.h"
#include <stdlib.h>
#include <stddef.h>

er_func_t *er_func_new(uint16_t me, uint16_t modid, 
                       uint8_t *code, size_t code_size) {
    er_func_t *func = er_xmalloc(sizeof(er_func_t));

    func->me = me;
    func->modid = modid;

    func->code = code;
    func->code_size = code_size;

    return func;
}

void er_func_delete(er_func_t *func) {
    free(func->code);
    free(func);
}

static void er_func_print(er_func_t *func, er_mod_t *mod) {
    ER_UNUSED(mod);

    fprintf(stderr, "  {\n");

    size_t at = 0;
    while (at < func->code_size) {
        fprintf(stderr, "    ");
        er_instr_print_with_address(at, func->code);
        at += er_instr_size(func->code[at]);
    }

    fprintf(stderr, "  }\n");
}

er_mod_t *er_mod_new(uint16_t me, er_const_t **consts, size_t n_consts, 
                     er_func_t **funcs, size_t n_funcs) {
    er_mod_t *mod = er_xmalloc(sizeof(er_mod_t));

    mod->me = me;

    mod->consts = consts;
    mod->n_consts = n_consts;

    mod->funcs = funcs;
    mod->n_funcs = n_funcs;

    return mod;
}

void er_mod_delete(er_mod_t *mod) {
    free(mod->consts);

    for (size_t i = 0; i < mod->n_funcs; i++) {
        er_func_delete(mod->funcs[i]);
    }
    free(mod->funcs);
    
    free(mod);
}

void er_mod_print(er_mod_t *mod) {
    fprintf(stderr, "CONSTS [%zu] {\n", mod->n_consts);
    
    for (size_t i = 0; i < mod->n_consts; i++) {
        fprintf(stderr, "  [%04zu] ", i);
        er_const_print(mod->consts[i]);
    }

    fprintf(stderr, "}\n");
    fprintf(stderr, "FUNCS [%zu] {\n", mod->n_funcs);

    for (size_t i = 0; i < mod->n_funcs; i++) {
        er_func_print(mod->funcs[i], mod);
    }

    fprintf(stderr, "}\n");
}
