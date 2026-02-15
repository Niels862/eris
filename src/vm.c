#include "vm.h"
#include "instruction.h"
#include "util.h"
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>

typedef union {
    int64_t i64;
} er_value_t;

typedef struct {
    er_value_t *stack;
    size_t stacksize;
    size_t sp;

    uint8_t const *code;
    size_t ip;

    bool halt;
} er_vm_t;

static void er_vm_init(er_vm_t *vm, uint8_t const *code) {
    vm->stacksize = 4096;
    vm->stack = er_xmalloc(vm->stacksize * sizeof(er_value_t));
    vm->sp = 0;
    vm->code = code;
    vm->ip = 0;
    vm->halt = false;
}

static void er_vm_destruct(er_vm_t *vm) {
    free(vm->stack);
}

static inline uint16_t er_read_u16_arg(er_vm_t *vm) {
    uint8_t hi = vm->code[++vm->ip];
    uint8_t lo = vm->code[++vm->ip];
    return (hi << 8) | lo;
}

#define ER_OPCODE_HANDLER(n) static inline void er_##n(er_vm_t *vm)

ER_OPCODE_HANDLER(NONE) {
    ER_UNUSED(vm);
}

ER_OPCODE_HANDLER(IPUSH_S16) {
    assert(vm->sp < vm->stacksize);

    int16_t arg = er_read_u16_arg(vm);
    vm->stack[vm->sp].i64 = arg;
    vm->sp++;
}

ER_OPCODE_HANDLER(IADD) {
    assert(vm->sp >= 2);

    vm->sp--;
    vm->stack[vm->sp - 1].i64 += vm->stack[vm->sp].i64;
}

ER_OPCODE_HANDLER(IPRINT) {
    assert(vm->sp >= 1);

    vm->sp--;
    printf("[VM]: %" PRId64 "\n", vm->stack[vm->sp].i64);
}

ER_OPCODE_HANDLER(HALT) {
    vm->halt = true;
}

static inline void er_dispatch(er_vm_t *vm, er_opcode_t opc) {
    switch (opc) {
        #define X(n) case ER_OPC_##n: er_##n(vm); break;
        ER_OPCODES(X);
        #undef X
        
        default:
            fprintf(stderr, "unhandled opcode: %d\n", opc);
            abort();
    }
}

void er_run(void) {
    static uint8_t const code[] = {
        ER_OPC_IPUSH_S16, 0, 60,
        ER_OPC_IPUSH_S16, 0, 9,
        ER_OPC_IADD,
        ER_OPC_IPRINT,
        ER_OPC_HALT,
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
