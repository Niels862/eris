#include "instruction.h"
#include <stdio.h>

#define ERIS_FORMAT_SIZE_NONE       1
#define ERIS_FORMAT_SIZE_CINDEX     3
#define ERIS_FORMAT_SIZE_S16        3

char *eris_instr_names[] = {
    ERIS_INSTRS(ERIS_INSTR_X_EXPAND_NAME)
};

eris_instr_format_t eris_instr_formats[] = {
    ERIS_INSTRS(ERIS_INSTR_X_EXPAND_FORMAT)
};

size_t eris_instr_sizes[] = {
    ERIS_INSTRS(ERIS_INSTR_X_EXPAND_SIZES)
};

size_t eris_disassemble_instr(uint8_t *code) {
    char *name = eris_instr_names[code[0]];
    eris_instr_format_t format = eris_instr_formats[code[0]];

    fprintf(stderr, "  %s", name);

    switch (format) {
        case ERIS_FORMAT_NONE: 
            fprintf(stderr, "\n");
            return 1;

        case ERIS_FORMAT_CINDEX: {
            eris_clabel_t label = code[1] | (code[2] << 8);
            fprintf(stderr, " c[%d]\n", label);
            return 3;
        }

        case ERIS_FORMAT_S16: {
            int16_t s16 = code[1] | (code[2] << 8);
            fprintf(stderr, " %d\n", s16);
            return 3;
        }

        default:
            fprintf(stderr, " <invalid format: %d>\n", format);
            return 1;
    }
}

void eris_disassemble(uint8_t *code, size_t size) {
    size_t i = 0;
    while (i < size) {
        i += eris_disassemble_instr(code + i);
    }
}
