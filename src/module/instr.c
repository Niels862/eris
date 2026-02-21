#include "module/instr.h"
#include <stddef.h>
#include <inttypes.h>

#define X(n, f) [ER_OPC_##n] = #n,
char const * const er_opcode_names[] = {
    ER_OPCODES(X)
};
#undef X

#define X(n, f) [ER_OPC_##n] = f,
er_instrfmt_t er_opcode_formats[] = {
    ER_OPCODES(X)
};
#undef X

char const *er_opcode_name(er_opcode_t opc) {
    if (opc < sizeof(er_opcode_names) / sizeof(*er_opcode_names)) {
        return er_opcode_names[opc];
    }
    return NULL;
}

er_instrfmt_t er_opcode_format(er_opcode_t opc) {
    if (opc < sizeof(er_opcode_names) / sizeof(*er_opcode_names)) {
        return er_opcode_formats[opc];
    }
    return ER_FMT_INVALID;
}

size_t er_instr_size(er_opcode_t opc) {
    er_instrfmt_t fmt = er_opcode_format(opc);
    int size = er_format_size(fmt);

    if (size < 0) {
        return 1;
    } else {
        return 1 + size;
    }
}

int er_format_size(er_instrfmt_t fmt) {
    switch (fmt) {
        case ER_OPC_NONE:
            return 0;

        case ER_FMT_S16:
        case ER_FMT_INDEX:
        case ER_FMT_JUMP:
            return 2;

        case ER_FMT_INVALID:
            break;
    }

    return -1;
}

void er_instr_print(size_t at, char const *buf, FILE *file) {
    er_opcode_t opc = buf[at];
    char const *name = er_opcode_name(opc);
    er_instrfmt_t fmt = er_opcode_format(opc);

    switch (fmt) {
        case ER_FMT_NONE: {
            fprintf(file, "%s\n", name);
            break;
        }

        case ER_FMT_S16: {
            int16_t value = 0;
            fprintf(file, "%s %" PRId16 "\n", name, value);
            break;
        }

        case ER_FMT_INDEX: {
            uint16_t value = 0;
            fprintf(file, "%s #%" PRIu16 "\n", name, value);
            break;
        }

        case ER_FMT_JUMP: {
            int16_t offset = 0;
            size_t address = at + offset;
            fprintf(file, "%s 0x%04zX\n", name, address);
            break;
        }

        case ER_FMT_INVALID: {
            fprintf(file, "(unknown)\n");
            break;
        }
    }
}

void er_instr_print_with_address(size_t at, char const *buf, FILE *file) {
    fprintf(file, "%04zX: ", at);
    er_instr_print(at, buf, file);
}
