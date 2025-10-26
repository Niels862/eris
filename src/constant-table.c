#include "constant-table.h"
#include "instruction.h"
#include "ctk/string-span.h"
#include <stdio.h>

const size_t eris_centry_size[] = {
    [ERIS_CONST_FUNCTION]       = sizeof(eris_const_function_t),
    [ERIS_CONST_STR]            = sizeof(eris_const_str_t),
};

const size_t eris_centry_align[] = {
    [ERIS_CONST_FUNCTION]       = sizeof(uint32_t),
    [ERIS_CONST_STR]            = sizeof(uint16_t),
};

void eris_const_write(void *vc, uint8_t *code, void **ctable) {
    switch (*(uint8_t *)vc) {
        case ERIS_CONST_FUNCTION: {
            eris_const_function_t *c = vc;
            eris_const_str_t *cs = ctable[c->name];

            fprintf(stderr, "<function %.*s> {\n", cs->size, cs->data);
            eris_disassemble(code + c->codestart, c->codesize);
            fprintf(stderr, "}\n");
            break;
        }

        case ERIS_CONST_STR: {
            eris_const_str_t *c = vc;
            fprintf(stderr, "<str[%d]> ", c->size);
            
            ctk_strspan_t span;
            ctk_strspan_init(&span, c->data, c->data + c->size);

            ctk_strspan_write_repr(&span, stderr);
            fprintf(stderr, "\n");
            break;
        }
    }
}

void eris_ctable_write(void **ctable, uint32_t ctablesize, uint8_t *code) {
    for (size_t i = 0; i < ctablesize; i++) {
        void *c = ctable[i];
        fprintf(stderr, "[%ld] ", i);
        eris_const_write(c, code, ctable);
    }
}
