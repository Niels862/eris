#ifndef ERIS_SYMBOL_H
#define ERIS_SYMBOL_H

#include "frontend/ast.h"
#include "ctk/rtti.h"

#define ERIS_SYMS(X) \
        X(BASE, sym) \
        X(FUNCTION, sym_function) \
        X(CLASS, sym_class)

#define ERIS_SYM_X_EXPAND_ENUM(e, t) CTK_SYM_##e,

#define ERIS_SYM_X_EXPAND_RTTI_DECLARE(e, t) CTK_RTTI_DECL(eris, t)
#define ERIS_SYM_X_EXPAND_RTTI_DEFINE(e, t) CTK_RTTI_DEFN(eris, t)

typedef enum {
    ERIS_SYMS(ERIS_SYM_X_EXPAND_ENUM)
} ctk_symkind_t;

typedef struct {
    ctk_rtti_t *meta;
    ctk_token_t *name;
} eris_sym_t;

typedef struct {
    eris_sym_t sym;
} eris_sym_function_t;

typedef struct {
    eris_sym_t sym;
} eris_sym_class_t;

ERIS_SYMS(ERIS_SYM_X_EXPAND_RTTI_DECLARE)

#endif
