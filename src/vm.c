#include "vm.h"
#include "instruction.h"
#include "consttable.h"
#include "util.h"
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>

typedef enum {
    ER_S64,
} er_typetag_t;

typedef union {
    int64_t s64;
} er_value_t;

typedef struct {
    size_t bp;
} er_callframe_t;

typedef struct {
    er_value_t *stack;
    uint8_t *stacktags; // = er_typetag_t *
    size_t stackcap;
    size_t sp;

    er_callframe_t *frames;
    size_t framescap;
    size_t fp;

    uint8_t const *code;
    size_t ip;

    er_const_t **consttab;
    size_t nconsts;

    bool halt;
} er_vm_t;

static void er_vm_init(er_vm_t *vm, uint8_t const *code) {
    vm->stackcap = 4096;
    vm->stack = er_xmalloc(vm->stackcap * sizeof(er_value_t));
    vm->stacktags = er_xmalloc(vm->stackcap * sizeof(uint8_t));
    vm->sp = 0;

    vm->framescap = 32;
    vm->frames = er_xmalloc(vm->framescap * sizeof(er_callframe_t));
    vm->fp = 0;

    vm->code = code;
    vm->ip = 0;

    vm->consttab = NULL;
    vm->nconsts = 0;

    vm->halt = false;
}

static void er_vm_destruct(er_vm_t *vm) {
    free(vm->stack);
    free(vm->stacktags);
    free(vm->frames);
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

ER_OPCODE_HANDLER(ILOAD_S16) {
    assert(vm->sp < vm->stackcap);

    int16_t arg = er_read_u16_arg(vm);
    vm->stack[vm->sp].s64 = arg;
    vm->stacktags[vm->sp] = ER_S64;
    vm->sp++;
}

ER_OPCODE_HANDLER(ILOAD_CONST) {
    uint16_t arg = er_read_u16_arg(vm);
    assert(arg < vm->nconsts);

    er_const_t *c = vm->consttab[arg];
    assert(c->tag == ER_CONST_S64);

    vm->stack[vm->sp].s64 = ((er_const_s64_t *)c)->s64;
    vm->stacktags[vm->sp] = ER_S64;
    vm->sp++;
}

ER_OPCODE_HANDLER(IADD) {
    assert(vm->sp >= 2);
    assert(vm->stacktags[vm->sp - 1] == ER_S64);
    assert(vm->stacktags[vm->sp - 2] == ER_S64);

    vm->sp--;
    vm->stack[vm->sp - 1].s64 += vm->stack[vm->sp].s64;
}

ER_OPCODE_HANDLER(IPRINT) {
    assert(vm->sp >= 1);
    assert(vm->stacktags[vm->sp - 1] == ER_S64);

    vm->sp--;
    printf("[VM]: %" PRId64 "\n", vm->stack[vm->sp].s64);
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
    static er_const_s64_t c1 = {
        .tag = ER_CONST_S64,
        .s64 = 60,
    };

    static er_const_t *consttab[] = {
        (er_const_t *)&c1,
    };

    static uint8_t const code[] = {
        ER_OPC_ILOAD_CONST, 0, 0,
        ER_OPC_ILOAD_S16, 0, 9,
        ER_OPC_IADD,
        ER_OPC_IPRINT,
        ER_OPC_HALT,
    };

    er_vm_t vm;
    er_vm_init(&vm, code);

    vm.consttab = consttab;
    vm.nconsts = sizeof(consttab) / sizeof(*consttab);

    while (!vm.halt) {
        fprintf(stderr, "%04zx: %s\n", vm.ip, er_opcode_name(vm.code[vm.ip]));

        er_dispatch(&vm, vm.code[vm.ip]);

        vm.ip++;
    }

    er_vm_destruct(&vm);
}
