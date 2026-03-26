#include "runtime/vm.h"
#include "module/instr.h"
#include "module/const.h"
#include "util/list.h"
#include "util/alloc.h"
#include "util/error.h"
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>

typedef union {
    int64_t s64;
} er_value_t;

typedef struct {
    size_t bp;
} er_callframe_t;

typedef struct {
    er_value_t *data;
    size_t cap;
    size_t size;
} er_valuestack_t;

typedef struct {
    er_callframe_t *data;
    size_t cap;
    size_t size;
} er_callstack_t;

typedef struct {
    er_valuestack_t values;
    er_callstack_t frames;

    uint8_t const *code;
    size_t ip;

    er_const_t **consttab;
    size_t nconsts;

    bool halt;
} er_vm_t;

static void er_vm_init(er_vm_t *vm, uint8_t const *code) {
    ER_LIST_INIT(&vm->values, 4096);
    ER_LIST_INIT(&vm->frames, 64);
    
    vm->code = code;
    vm->ip = 0;

    vm->consttab = NULL;
    vm->nconsts = 0;

    vm->halt = false;
}

static void er_vm_destruct(er_vm_t *vm) {
    ER_UNUSED(vm);
}

static inline uint16_t er_read_u16_arg(er_vm_t *vm) {
    uint8_t hi = vm->code[++vm->ip];
    uint8_t lo = vm->code[++vm->ip];
    return (hi << 8) | lo;
}

#define ER_OPCODE_HANDLER(n) static inline void er_##n(er_vm_t *vm)

#define ER_UNIMPLEMENTED_OPCODE_HANDLER(n) \
        ER_OPCODE_HANDLER(n) { ER_UNUSED(vm); ER_UNIMPLEMENTED_FUNCTION(); }

ER_UNIMPLEMENTED_OPCODE_HANDLER(NONE)

ER_UNIMPLEMENTED_OPCODE_HANDLER(LOAD_LOCAL)

ER_UNIMPLEMENTED_OPCODE_HANDLER(STORE_LOCAL)

ER_UNIMPLEMENTED_OPCODE_HANDLER(LOAD_NULL)

ER_UNIMPLEMENTED_OPCODE_HANDLER(LOAD_TRUE)

ER_UNIMPLEMENTED_OPCODE_HANDLER(LOAD_FALSE)

ER_UNIMPLEMENTED_OPCODE_HANDLER(LOAD_INT)

ER_UNIMPLEMENTED_OPCODE_HANDLER(LOAD_CONST)

ER_UNIMPLEMENTED_OPCODE_HANDLER(POP)

ER_UNIMPLEMENTED_OPCODE_HANDLER(DUP_TOP)

ER_UNIMPLEMENTED_OPCODE_HANDLER(INVOKE)

ER_UNIMPLEMENTED_OPCODE_HANDLER(RETURN)

ER_UNIMPLEMENTED_OPCODE_HANDLER(ASSERT)

ER_UNIMPLEMENTED_OPCODE_HANDLER(JUMP_IF_TRUE)

ER_UNIMPLEMENTED_OPCODE_HANDLER(JUMP_IF_FALSE)

ER_UNIMPLEMENTED_OPCODE_HANDLER(JUMP)

ER_UNIMPLEMENTED_OPCODE_HANDLER(IADD)

ER_UNIMPLEMENTED_OPCODE_HANDLER(ISUB)

ER_UNIMPLEMENTED_OPCODE_HANDLER(EQUALS)

ER_UNIMPLEMENTED_OPCODE_HANDLER(NOT_EQUALS)

static inline void er_dispatch(er_vm_t *vm, er_opcode_t opc) {
    switch (opc) {
        #define X(n, f) case ER_OPC_##n: er_##n(vm); break;
        ER_OPCODES(X);
        #undef X
        
        default:
            fprintf(stderr, "unhandled opcode: %d\n", opc);
            abort();
    }
}

void er_run(er_mod_t **mods) {
    ER_UNUSED(mods);

    uint8_t code[] = {
        ER_OPC_LOAD_LOCAL,
    };

    er_vm_t vm;
    er_vm_init(&vm, code);

    while (!vm.halt) {
        fprintf(stderr, "%04zx: %s\n", vm.ip, er_opcode_name(vm.code[vm.ip]));

        er_dispatch(&vm, vm.code[vm.ip]);

        vm.ip++;
    }

    er_vm_destruct(&vm);
}
