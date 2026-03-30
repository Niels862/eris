#include "compiler/type.h"
#include "compiler/symbol.h"
#include <stdio.h>

static void er_type_print_internal(er_type_t *type) {
    switch (type->kind) {
        case ER_TYPE_CLASS: {
            er_typeclass_t *Class = &type->data.Class;
            er_sym_t *sym = Class->sym;
            fprintf(stderr, "%.*s", sym->name.len, sym->name.data);
            break;
        }

        case ER_TYPE_FUNC: {
            er_typefunc_t *Func = &type->data.Func;
            fprintf(stderr, "() -> ");
            er_type_print_internal(Func->rettype);
            break;
        }
    }
}

void er_type_print(er_type_t *type) {
    er_type_print_internal(type);
    fprintf(stderr, "\n");
}
