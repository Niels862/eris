#ifndef ERIS_INSTRUCTION_H
#define ERIS_INSTRUCTION_H

#include <stddef.h>
#include <stdint.h>

#define ERIS_INSTRS(X) \
        X(NOP,          nop,            NONE) \
        X(INVOKE,       invoke,         CINDEX) \
        X(IRETURN,      ireturn,        NONE) \
        X(IPUSH,        ipush,          S16) \
        X(POP,          pop,            NONE)

#define ERIS_INSTR_X_EXPAND_ENUM(e, s, f) \
        ERIS_INSTR_##e,

#define ERIS_INSTR_X_EXPAND_NAME(e, s, f) \
        [ERIS_INSTR_##e] = #s,

#define ERIS_INSTR_X_EXPAND_FORMAT(e, s, f) \
        [ERIS_INSTR_##e] = ERIS_FORMAT_##f,

#define ERIS_INSTR_X_EXPAND_SIZES(e, s, f) \
        [ERIS_INSTR_##e] = ERIS_FORMAT_SIZE_##f,

typedef enum {
    ERIS_INSTRS(ERIS_INSTR_X_EXPAND_ENUM)
} eris_instr_t;

typedef uint16_t eris_clabel_t;
typedef uint16_t eris_cindex_t;

typedef enum {
    ERIS_FORMAT_NONE,
    ERIS_FORMAT_CINDEX,
    ERIS_FORMAT_S16,
} eris_instr_format_t;

extern char *eris_instr_names[];
extern eris_instr_format_t eris_instr_formats[];
extern size_t eris_instr_sizes[];

size_t eris_disassemble_instr(uint8_t *code);

void eris_disassemble(uint8_t *code, size_t size);

static inline uint16_t eris_read_u16(uint8_t *buf) {
    return buf[0] | (buf[1] << 8);
}

#endif
