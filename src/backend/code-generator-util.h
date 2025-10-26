#ifndef CODE_GENERATOR_UTIL_H
#define CODE_GENERATOR_UTIL_H

#include "code-generator.h"
#include "instruction.h"
#include "constant-table.h"
#include "ctk/dynamic-array.h"
#include "ctk/list.h"
#include <string.h>

typedef struct eris_codegen_t eris_codegen_t;

typedef void(*eris_codegen_visit_t)(eris_codegen_t *gen, void *node);

struct eris_codegen_t {
    ctk_dynarr_t code;
    ctk_list_t ctable;
    ctk_pool_t cpool;
    eris_codegen_visit_t *visitors;
};

static void eris_codegen_init(eris_codegen_t *gen, eris_codegen_visit_t *visitors) {
    ctk_dynarr_init(&gen->code, 16);
    ctk_list_init(&gen->ctable, 16);
    ctk_pool_init(&gen->cpool);
    gen->visitors = visitors;
}

static void eris_codegen_move_to_module(eris_codegen_t *gen, 
                                        eris_module_t *mod) {
    eris_module_init(mod, 
        ctk_dynarr_move(&gen->code), gen->code.size,
        ctk_list_move(&gen->ctable), ctk_list_size(&gen->ctable), 
        &gen->cpool);
}

static void *eris_codegen_add_const(eris_codegen_t *gen, 
                                    eris_centry_kind_t kind, 
                                    size_t size,
                                    eris_cindex_t *idx) {
    size_t csize = eris_centry_size[kind] + size;
    size_t calign = eris_centry_align[kind];

    *idx = ctk_list_size(&gen->ctable);

    void *c = ctk_pool_alloc_aligned(&gen->cpool, csize, calign);
    ctk_list_add(&gen->ctable, c);

    *(uint8_t *)c = kind;

    assert((uintptr_t)c % calign == 0);

    return c;
}

static eris_cindex_t eris_codegen_add_str(eris_codegen_t *gen, 
                                          char *s, size_t size) {
    eris_cindex_t idx;
    eris_const_str_t *c = eris_codegen_add_const(gen, ERIS_CONST_STR, 
                                                 size, &idx);
    
    c->size = size;
    memcpy(c->data, s, size);

    return idx;
}

static void eris_emit_instr(eris_codegen_t *gen, eris_instr_t instr) {
    uint8_t *p = ctk_dynarr_add(&gen->code, 1);
    p[0] = instr;
}

static void eris_emit_instr_with_cindex(eris_codegen_t *gen, 
                                        eris_instr_t instr, 
                                        eris_cindex_t index) {
    uint8_t *p = ctk_dynarr_add(&gen->code, 3);
    p[0] = instr;
    p[1] = index & 0xFF;
    p[2] = (index >> 8) & 0xFF;
}

static void eris_emit_instr_with_s16(eris_codegen_t *gen, eris_instr_t instr, 
                                     int16_t s16) {
    uint16_t u16 = s16;
    
    uint8_t *p = ctk_dynarr_add(&gen->code, 3);
    p[0] = instr;
    p[1] = u16 & 0xFF;
    p[2] = (u16 >> 8) & 0xFF;            
}

#define ERIS_EMITTER_NONE(e, s) \
        inline static void eris_emit_##s(eris_codegen_t *gen) \
            { eris_emit_instr(gen, ERIS_INSTR_##e); }

#define ERIS_EMITTER_CINDEX(e, s) \
        inline static void eris_emit_##s(eris_codegen_t *gen, \
                                         eris_clabel_t label) \
            { eris_emit_instr_with_cindex(gen, ERIS_INSTR_##e, label); }

#define ERIS_EMITTER_S16(e, s) \
        inline static void eris_emit_##s(eris_codegen_t *gen, \
                                         eris_clabel_t label) \
            { eris_emit_instr_with_s16(gen, ERIS_INSTR_##e, label); }

#define ERIS_INSTR_X_EXPAND_EMIT(e, s, f) ERIS_EMITTER_##f(e, s)

ERIS_INSTRS(ERIS_INSTR_X_EXPAND_EMIT)

static void eris_codegen_dispatch(eris_codegen_t *gen, void *node) {
    ctk_rtti_t *meta = CTK_RTTI_META(node);
    assert(meta->id != 0);
    assert(eris_node_instanceof(node));

    eris_codegen_visit_t visit = gen->visitors[meta->id];
    assert(visit != NULL);

    visit(gen, node);
}

#endif

