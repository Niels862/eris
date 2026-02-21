#include "module/instr.h"
#include "runtime/vm.h"
#include "util/error.h"

int main(int argc, char *argv[]) {
    ER_UNUSED(argc), ER_UNUSED(argv);

    char code[] = {
        ER_OPC_LOAD_CONST, 0, 2,
        ER_OPC_LOAD_LOCAL, 0, 1,
        ER_OPC_IADD,
        ER_OPC_RETURN,
        ER_OPC_JUMP,
    };

    size_t at = 0;
    while (at < sizeof(code)) {
        er_instr_print_with_address(at, code, stderr);
        at += er_instr_size(code[at]);
    }

    return 0;
}
