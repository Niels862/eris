#include "module/mod.h"
#include "module/instr.h"
#include "util/alloc.h"
#include <stdlib.h>
#include <stddef.h>

er_func_t *er_func_new(er_constidx_t name, uint16_t modid, 
                       uint8_t *code, size_t code_size, 
                       uint16_t n_args, uint16_t n_locals, uint16_t n_temps) {
    er_func_t *func = er_xmalloc(sizeof(er_func_t));

    func->name = name;
    func->modid = modid;

    func->code = code;
    func->code_size = code_size;

    func->n_args = n_args;
    func->n_locals = n_locals;
    func->n_temps = n_temps;

    return func;
}

void er_func_delete(er_func_t *func) {
    free(func->code);
    free(func);
}

static void er_func_print(er_func_t *func, er_mod_t *mod) {
    ER_UNUSED(mod);

    er_const_str_t *name = (er_const_str_t *)mod->consts[func->name];

    fprintf(stderr, "  function %.*s (#%" PRIu16 ":#%" PRIu16 ") "
            "with %" PRIu16 " args, %" PRIu16 " locals, %" PRIu16 " temps {\n", 
            name->len, name->data, mod->me, func->name,
            func->n_args, func->n_locals, func->n_temps);

    size_t at = 0;
    while (at < func->code_size) {
        fprintf(stderr, "    ");
        er_instr_print_with_address(at, func->code);
        at += er_instr_size(func->code[at]);
    }

    fprintf(stderr, "  }\n");
}

er_mod_t *er_mod_new(er_constidx_t me, uint16_t id, 
                     er_const_t **consts, size_t n_consts, 
                     er_func_t **funcs, size_t n_funcs) {
    er_mod_t *mod = er_xmalloc(sizeof(er_mod_t));

    mod->me = me;
    mod->id = id;

    mod->consts = consts;
    mod->n_consts = n_consts;

    mod->funcs = funcs;
    mod->n_funcs = n_funcs;

    return mod;
}

void er_mod_delete(er_mod_t *mod) {
    for (size_t i = 0; i < mod->n_consts; i++) {
        free(mod->consts[i]);
    }
    free(mod->consts);

    for (size_t i = 0; i < mod->n_funcs; i++) {
        er_func_delete(mod->funcs[i]);
    }
    free(mod->funcs);
    
    free(mod);
}

void er_mod_print(er_mod_t *mod) {
    er_const_modref_t *modref = (er_const_modref_t *)mod->consts[mod->me];
    er_const_str_t *name = (er_const_str_t *)mod->consts[modref->name];

    fprintf(stderr, "Module %.*s ($%" PRIu16 ")", 
            name->len, name->data, mod->id);
    fprintf(stderr, " with consts[%zu] {\n", mod->n_consts);
    
    for (size_t i = 0; i < mod->n_consts; i++) {
        fprintf(stderr, "  [%04zu] ", i);
        er_const_print(mod->consts[i]);
    }

    fprintf(stderr, "} and with funcs[%zu] {\n", mod->n_funcs);

    for (size_t i = 0; i < mod->n_funcs; i++) {
        er_func_print(mod->funcs[i], mod);
    }

    fprintf(stderr, "}\n");
}
