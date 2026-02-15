#ifndef ER_INSTRUCTION_H
#define ER_INSTRUCTION_H

#define ER_OPCODES(X) \
    X(NONE) \
    X(ILOAD_S16) \
    X(ILOAD_CONST) \
    X(IADD) \
    X(IPRINT) \
    X(HALT)

#define X(n) ER_OPC_##n,
typedef enum {
    ER_OPCODES(X)
} er_opcode_t;
#undef X

char const *er_opcode_name(er_opcode_t opc);

#endif
