#include "runtime/virtual-machine.h"
#include "instruction.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    eris_const_function_t *func;
    uint32_t base;
    uint32_t top;
    uint32_t instr;
} eris_stackframe_t;

typedef struct {
    eris_stackframe_t frames[16]; // TEMP: max calling depth of 16
    uint64_t stack[256]; // TEMP: max stack size of 256
    uint32_t top;
} eris_vm_t;

void eris_run(eris_module_t *mod, eris_const_function_t *entry) {
    eris_vm_t vm = {
        .frames = {
            [0] = {
                .func = entry,
                .base = 0,
                .instr = entry->codestart,
            }
        },
        .top = 0,
    };

    while (true) {
        eris_stackframe_t *curr = &vm.frames[vm.top];
        eris_instr_t instr = mod->code[curr->instr];

        fprintf(stderr, "%6d ", curr->instr);
        eris_disassemble_instr(&mod->code[curr->instr]);

        uint8_t *arg = &mod->code[curr->instr + 1];
        curr->instr += eris_instr_sizes[instr];

        switch (instr) {
            case ERIS_INSTR_NOP: {
                break;
            }

            case ERIS_INSTR_INVOKE: {
                break;
            }

            case ERIS_INSTR_IRETURN: {
                int64_t value = ((int64_t *)vm.stack)[curr->top - 1];

                if (vm.top > 0) {
                    // TODO
                } else {
                    fprintf(stderr, "Program returned %ld on exit\n", value);
                    return;
                }
                break;
            }

            case ERIS_INSTR_IPUSH: {
                int16_t s16 = (int16_t)eris_read_u16(arg);
                ((int64_t *)vm.stack)[curr->top] = s16;
                curr->top++;
                break;
            }

            case ERIS_INSTR_POP: {
                curr->top--;
                break;
            }
        }
    }
}
