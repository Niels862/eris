#include "code-generator.h"
#include "ast.h"
#include "instruction.h"
#include "ctk/rtti.h"
#include <assert.h>

typedef struct eris_codegen_t eris_codegen_t;

typedef void(*eris_codegen_visit_t)(eris_codegen_t *gen, void *node);

struct eris_codegen_t {
    ctk_dynarr_t code;
    eris_codegen_visit_t *visitors;
};

static void eris_codegen_init(eris_codegen_t *gen, eris_codegen_visit_t *visitors) {
    ctk_dynarr_init(&gen->code, 16);
    gen->visitors = visitors;
}

static void eris_codegen_move_to_module(eris_codegen_t *gen, 
                                        eris_module_t *mod) {
    mod->code = ctk_dynarr_move(&gen->code);
    mod->codesize = gen->code.size;
}

static void eris_emit_instr(eris_codegen_t *gen, eris_instr_t instr) {
    uint8_t *p = ctk_dynarr_add(&gen->code, 1);
    p[0] = instr;
}

static void eris_emit_instr_with_clabel(eris_codegen_t *gen, 
                                        eris_instr_t instr, 
                                        eris_clabel_t label) {
    uint8_t *p = ctk_dynarr_add(&gen->code, 3);
    p[0] = instr;
    p[1] = label & 0xFF;
    p[2] = (label >> 8) & 0xFF;
}

#define ERIS_FNONE(e, s) \
        inline static void eris_emit_##s(eris_codegen_t *gen) \
            { eris_emit_instr(gen, ERIS_INSTR_##e); }

#define ERIS_FCLABEL(e, s) \
        inline static void eris_emit_##s(eris_codegen_t *gen, \
                                         eris_clabel_t label) \
            { eris_emit_instr_with_clabel(gen, ERIS_INSTR_##e, label); }

#define ERIS_INSTR_X_EXPAND_EMIT(e, s, f) f(e, s)

ERIS_INSTRS(ERIS_INSTR_X_EXPAND_EMIT)

static void eris_codegen_dispatch(eris_codegen_t *gen, void *node) {
    ctk_rtti_t *meta = CTK_RTTI_META(node);
    assert(meta->id != 0);
    assert(eris_node_instanceof(node));

    eris_codegen_visit_t visit = gen->visitors[meta->id];
    assert(visit != NULL);

    visit(gen, node);
}

static void eris_codegen_source(eris_codegen_t *gen, void *vnode) {
    eris_node_source_t *node = eris_node_source_dyncast(vnode);

    eris_emit_invoke(gen, 0);

    (void)node;
}

void eris_codegen(eris_node_source_t *root, eris_module_t *mod) {
    static eris_codegen_visit_t visitors[] = {
        [CTK_NODE_SOURCE]           = &eris_codegen_source,
    };
    
    eris_codegen_t gen;
    eris_codegen_init(&gen, visitors);

    eris_codegen_dispatch(&gen, root);

    eris_codegen_move_to_module(&gen, mod);
}
