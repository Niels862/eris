#include "runtime/vm.h"
#include "module/instr.h"
#include "module/const.h"
#include "util/list.h"
#include "util/serial.h"
#include "util/alloc.h"
#include "util/error.h"
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>

typedef union {
    uint64_t u64;
    int64_t s64;
} er_value_t;

typedef struct {
    er_func_t *func;

    uint8_t const *code;
    size_t ip;
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

    er_func_t *func;
    uint8_t const *code;
    size_t ip;

    er_const_t **consttab;
    size_t nconsts;

    bool halt;
} er_vm_t;

static void er_vm_init(er_vm_t *vm) {
    ER_LIST_INIT(&vm->values, 4096);
    ER_LIST_INIT(&vm->frames, 64);
    
    vm->func    = NULL;
    vm->code    = NULL;
    vm->ip      = 0;

    vm->consttab = NULL;
    vm->nconsts = 0;

    vm->halt = false;
}

static void er_vm_destruct(er_vm_t *vm) {
    ER_UNUSED(vm);
}

static void er_stack_print(er_valuestack_t *values, size_t start) {
    fprintf(stderr, "  Stack state {\n");

    for (size_t i = start; i < values->size; i++) {
        er_value_t val = values->data[i];
        fprintf(stderr, "    [%04zX] = 0x%08" PRIX64 " (%" PRId64 ")\n", 
                i, val.u64, val.s64);
    }

    fprintf(stderr, "  }\n");
}

static void er_vm_print(er_vm_t *vm) {
    fprintf(stderr, "Eris VM State {\n");

    if (vm->func != NULL) {
        fprintf(stderr, "  Currently in @%" PRIu16 ":#%" PRIu16 "\n", 
                vm->func->modid, vm->func->name);
    } else {
        fprintf(stderr, "  FUNC is unset\n");
    }
    fprintf(stderr, "  %zu frames on callstack\n", vm->frames.size);

    er_stack_print(&vm->values, 0);

    fprintf(stderr, "}\n");
}

static inline uint16_t er_read_arg(er_vm_t *vm) {
    uint16_t arg = er_read_u16(&vm->code[vm->ip]);
    vm->ip += 2;
    return arg;
}

static void er_call(er_vm_t *vm, er_func_t *func) {
    ER_LIST_RESERVE_ONE(&vm->frames);

    er_callframe_t *frame = &vm->frames.data[vm->frames.size];
    vm->frames.size++;

    frame->func = vm->func;
    frame->code = vm->code;
    frame->ip   = vm->ip;

    vm->func    = func;
    vm->code    = func->code;
    vm->ip      = 0;
}

static void er_return(er_vm_t *vm) {
    assert(vm->frames.size > 0);

    vm->frames.size--;
    er_callframe_t *frame = &vm->frames.data[vm->frames.size];

    vm->func    = frame->func;
    vm->code    = frame->code;
    vm->ip      = frame->ip;

    if (vm->func == NULL) {
        vm->halt = true;
    }
}

static void er_call_entry(er_vm_t *vm, er_mod_t *mod) {
    assert(mod->n_funcs > 0);

    // FIXME: Find entry function if it exists 
    er_func_t *entry = mod->funcs[0];

    er_call(vm, entry);
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

ER_OPCODE_HANDLER(LOAD_INT) {
    int64_t s64 = (int16_t)er_read_arg(vm);
    vm->values.data[vm->values.size++].s64 = s64;
}

ER_UNIMPLEMENTED_OPCODE_HANDLER(LOAD_CONST)

ER_UNIMPLEMENTED_OPCODE_HANDLER(POP)

ER_UNIMPLEMENTED_OPCODE_HANDLER(DUP_TOP)

ER_UNIMPLEMENTED_OPCODE_HANDLER(INVOKE)

ER_OPCODE_HANDLER(RETURN) {
    er_return(vm);
}

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
            ER_UNHANDLED_SWITCH_VALUE("%" PRIu8, opc);
    }
}

static void er_loop(er_vm_t *vm) {
    while (!vm->halt) {
        er_instr_print_with_address(vm->ip, vm->code);

        er_opcode_t opc = vm->code[vm->ip];
        vm->ip++;

        er_dispatch(vm, opc);

        er_vm_print(vm);
    }
}

void er_run(er_mod_t **mods) {
    er_vm_t vm;
    er_vm_init(&vm);

    er_call_entry(&vm, mods[0]);
    er_loop(&vm);

    fprintf(stderr, "On exit: ");
    er_vm_print(&vm);

    er_vm_destruct(&vm);
}
