#include "runtime/vm.h"
#include "module/instr.h"
#include "module/const.h"
#include "util/alloc.h"
#include "util/error.h"
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

ER_OPCODE_HANDLER(LOAD_LOCAL)  {
    ER_UNUSED(vm);
}

ER_OPCODE_HANDLER(STORE_LOCAL)  {
    ER_UNUSED(vm);
}

ER_OPCODE_HANDLER(LOAD_NULL)  {
    ER_UNUSED(vm);
}

ER_OPCODE_HANDLER(LOAD_TRUE)  {
    ER_UNUSED(vm);
}

ER_OPCODE_HANDLER(LOAD_FALSE)  {
    ER_UNUSED(vm);
}

ER_OPCODE_HANDLER(LOAD_CONST)  {
    ER_UNUSED(vm);
}

ER_OPCODE_HANDLER(POP)  {
    ER_UNUSED(vm);
}

ER_OPCODE_HANDLER(DUP_TOP)  {
    ER_UNUSED(vm);
}

ER_OPCODE_HANDLER(INVOKE)  {
    ER_UNUSED(vm);
}

ER_OPCODE_HANDLER(RETURN)  {
    ER_UNUSED(vm);
}

ER_OPCODE_HANDLER(ASSERT)  {
    ER_UNUSED(vm);
}

ER_OPCODE_HANDLER(JUMP_IF_TRUE)  {
    ER_UNUSED(vm);
}

ER_OPCODE_HANDLER(JUMP_IF_FALSE)  {
    ER_UNUSED(vm);
}

ER_OPCODE_HANDLER(JUMP)  {
    ER_UNUSED(vm);
}

ER_OPCODE_HANDLER(IADD)  {
    ER_UNUSED(vm);
}

ER_OPCODE_HANDLER(ISUB)  {
    ER_UNUSED(vm);
}

ER_OPCODE_HANDLER(EQUALS)  {
    ER_UNUSED(vm);
}

ER_OPCODE_HANDLER(NOT_EQUALS)  {
    ER_UNUSED(vm);
}

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

void er_run(void) {
    static er_const_s64_t c1 = {
        .tag = ER_CONST_S64,
        .s64 = 60,
    };

    static er_const_t *consttab[] = {
        (er_const_t *)&c1,
    };

    static uint8_t const code[] = {
        0,
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
