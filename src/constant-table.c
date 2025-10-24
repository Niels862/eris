#include "constant-table.h"
#include "instruction.h"
#include <stdio.h>

const size_t eris_centry_size[] = {
    [ERIS_CONST_FUNCTION]       = sizeof(eris_const_function_t),
};

const size_t eris_centry_align[] = {
    [ERIS_CONST_FUNCTION]       = sizeof(uint32_t),
};

void eris_const_write(void *vc, uint8_t *code) {
    switch (*(uint8_t *)vc) {
        case ERIS_CONST_FUNCTION: {
            eris_const_function_t *c = vc;
            fprintf(stderr, "<function> {\n");
            eris_disassemble(code + c->codestart, c->codesize);
            fprintf(stderr, "}\n");
        }
    }
}

void eris_ctable_write(uint32_t *ctable, uint32_t ctablesize, 
                       uint8_t *cdata, uint8_t *code) {
    for (size_t i = 0; i < ctablesize; i++) {
        uint32_t idx = ctable[i];

        fprintf(stderr, "[%ld] ", i);
        eris_const_write(cdata + idx, code);
    }
}
