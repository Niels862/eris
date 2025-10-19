#include "code-generator.h"

void eris_codegen_init(eris_codegen_t *gen) {
    ctk_dynarr_init(&gen->code, 16);
}

void eris_codegen_destruct(eris_codegen_t *gen) {
    ctk_dynarr_destruct(&gen->code);
}
