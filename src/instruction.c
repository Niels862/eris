#include "instruction.h"
#include <stdio.h>

char *eris_instr_names[] = {
    ERIS_INSTRS(ERIS_INSTR_X_EXPAND_NAME)
};

size_t eris_disassemble_instr(uint8_t *code) {
    char *name = eris_instr_names[code[0]];
    
    fprintf(stderr, "  %s\n", name);

    return 1;
}

void eris_disassemble(uint8_t *code, size_t size) {
    size_t i = 0;
    while (i < size) {
        i += eris_disassemble_instr(code + i);
    }
}
