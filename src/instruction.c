#include "instruction.h"
#include <stddef.h>

#define X(n) #n,
char const * const er_opcode_names[] = {
    ER_OPCODES(X)
};
#undef X

char const *er_opcode_name(er_opcode_t opc) {
    if (opc < sizeof(er_opcode_names) / sizeof(*er_opcode_names)) {
        return er_opcode_names[opc];
    }
    return NULL;
}
