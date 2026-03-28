#include "module/instr.h"
#include "util/serial.h"
#include <stdio.h>
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

void er_instr_print(size_t at, uint8_t const *code) {
    er_opcode_t opc = code[at];
    char const *name = er_opcode_name(opc);
    er_instrfmt_t fmt = er_opcode_format(opc);
    uint8_t const *arg = code + at + 1;

    switch (fmt) {
        case ER_FMT_NONE: {
            fprintf(stderr, "%s\n", name);
            break;
        }

        case ER_FMT_S16: {
            int16_t value = er_read_u16(arg);
            fprintf(stderr, "%s %" PRId16 "\n", name, value);
            break;
        }

        case ER_FMT_INDEX: {
            uint16_t value = er_read_u16(arg);
            fprintf(stderr, "%s #%" PRIu16 "\n", name, value);
            break;
        }

        case ER_FMT_JUMP: {
            int16_t offset = er_read_u16(arg);
            size_t address = at + offset;
            fprintf(stderr, "%s 0x%04zX\n", name, address);
            break;
        }

        case ER_FMT_INVALID: {
            fprintf(stderr, "(unknown)\n");
            break;
        }
    }
}

void er_instr_print_with_address(size_t at, uint8_t const *code) {
    fprintf(stderr, "%04zX: ", at);
    er_instr_print(at, code);
}
