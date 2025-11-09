#include "frontend/symbol.h"

static ctk_rtti_t eris_sym_rtti = {
    .super = &ctk_rtti_base,
    .name = "sym-base",
    .id = CTK_SYM_BASE,
};

static ctk_rtti_t eris_sym_function_rtti = {
    .super = &eris_sym_rtti,
    .name = "sym-function",
    .id = CTK_SYM_FUNCTION,
};

static ctk_rtti_t eris_sym_class_rtti = {
    .super = &eris_sym_rtti,
    .name = "node-class",
    .id = CTK_SYM_CLASS,
};

ERIS_SYMS(ERIS_SYM_X_EXPAND_RTTI_DEFINE)
