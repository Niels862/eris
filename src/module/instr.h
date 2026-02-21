#ifndef ER_INSTRUCTION_H
#define ER_INSTRUCTION_H

#include <stdio.h>

typedef enum {
    ER_FMT_NONE,
    ER_FMT_S16,
    ER_FMT_INDEX,
    ER_FMT_JUMP,

    ER_FMT_INVALID,
} er_instrfmt_t;

#define ER_OPCODES(X) \
    X(NONE,             ER_FMT_NONE) \
    X(LOAD_LOCAL,       ER_FMT_S16) \
    X(STORE_LOCAL,      ER_FMT_S16) \
    X(LOAD_NULL,        ER_FMT_NONE) \
    X(LOAD_TRUE,        ER_FMT_NONE) \
    X(LOAD_FALSE,       ER_FMT_NONE) \
    X(LOAD_CONST,       ER_FMT_INDEX) \
    X(POP,              ER_FMT_NONE) \
    X(DUP_TOP,          ER_FMT_NONE) \
    X(INVOKE,           ER_FMT_INDEX) \
    X(RETURN,           ER_FMT_NONE) \
    X(ASSERT,           ER_FMT_NONE) \
    X(JUMP_IF_TRUE,     ER_FMT_JUMP) \
    X(JUMP_IF_FALSE,    ER_FMT_JUMP) \
    X(JUMP,             ER_FMT_JUMP) \
    X(IADD,             ER_FMT_NONE) \
    X(ISUB,             ER_FMT_NONE) \
    X(EQUALS,           ER_FMT_NONE) \
    X(NOT_EQUALS,       ER_FMT_NONE)

#define X(n, f) ER_OPC_##n,
typedef enum {
    ER_OPCODES(X)
} er_opcode_t;
#undef X

char const *er_opcode_name(er_opcode_t opc);

er_instrfmt_t er_opcode_format(er_opcode_t opc);

size_t er_instr_size(er_opcode_t opc);

int er_format_size(er_instrfmt_t fmt);

void er_instr_print(size_t at, char const *buf, FILE *file);

void er_instr_print_with_address(size_t at, char const *buf, FILE *file);

#endif
