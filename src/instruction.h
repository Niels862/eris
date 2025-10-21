#ifndef ERIS_INSTRUCTION_H
#define ERIS_INSTRUCTION_H

#include <stddef.h>
#include <stdint.h>

#define ERIS_INSTRS_OTHER(X) \
        X(NOP,          nop,            ERIS_FNONE) \
        X(INVOKE,       invoke,         ERIS_FCLABEL) \
        X(IRETURN,      ireturn,        ERIS_FNONE)

#define ERIS_INSTRS(X) \
        ERIS_INSTRS_OTHER(X)

#define ERIS_INSTR_X_EXPAND_ENUM(e, s, f) ERIS_INSTR_##e,
#define ERIS_INSTR_X_EXPAND_NAME(e, s, f) [ERIS_INSTR_##e] = #s,

typedef enum {
    ERIS_INSTRS(ERIS_INSTR_X_EXPAND_ENUM)
} eris_instr_t;

typedef uint16_t eris_clabel_t;
typedef uint16_t eris_cindex_t;

extern char *eris_instr_names[];

size_t eris_disassemble_instr(uint8_t *code);

void eris_disassemble(uint8_t *code, size_t size);

#endif
