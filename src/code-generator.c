#include "code-generator.h"
#include "ast.h"
#include "ctk/rtti.h"
#include <assert.h>

void eris_codegen_init(eris_codegen_t *gen, eris_codegen_visit_t *visitors) {
    ctk_dynarr_init(&gen->code, 16);
    gen->visitors = visitors;
}

void eris_codegen_destruct(eris_codegen_t *gen) {
    ctk_dynarr_destruct(&gen->code);
}

void eris_codegen_dispatch(eris_codegen_t *gen, void *node) {
    ctk_rtti_t *meta = CTK_RTTI_META(node);
    assert(meta->id != 0);
    assert(eris_node_instanceof(node));

    eris_codegen_visit_t visit = gen->visitors[meta->id];
    assert(visit != NULL);

    visit(gen, node);
}
