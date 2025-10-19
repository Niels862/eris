#ifndef ERIS_INSTRUCTION_H
#define ERIS_INSTRUCTION_H

#define ERIS_INSTRS_OTHER(X) \
        X(NOP, nop) \
        X(INVOKE, invoke) \
        X(IRETURN, ireturn)

#define ERIS_INSTRS(X) \
        ERIS_INSTRS_OTHER(X)

#define ERIS_INSTR_X_EXPAND_ENUM(e, s) ERIS_INSTR_##e,
#define ERIS_INSTR_X_EXPAND_NAME(e, s) [ERIS_INSTR_##e] = #s,

typedef enum {
    ERIS_INSTRS(ERIS_INSTR_X_EXPAND_ENUM)
} eris_instr_t;

extern char *eris_instr_names[];

#endif
