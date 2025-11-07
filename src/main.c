#include "frontend/source.h"
#include "runtime/instruction.h"
#include "runtime/constant-table.h"
#include "runtime/loader.h"
#include "runtime/virtual-machine.h"
#include <stdio.h>

void eris_init(void) {
    ctk_tokenkind_set_name_table(eris_token_names);
}

int main(int argc, char *argv[]) {
    eris_init();

    if (argc != 2) {
        fprintf(stderr, "Usage: %s [filename]\n", argv[0]);
        return 1;
    }
    
    int res = 1;

    ctk_zstr_t filename = argv[1];
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Could not open file: `%s`\n", filename);
        goto done;
    }

    eris_module_t mod;
    eris_module_init_empty(&mod);

    eris_codesrc_t csrc;
    eris_codesrc_init(&csrc, filename, file, &mod);

    fclose(file);

    eris_codesrc_lex(&csrc);
    eris_codesrc_parse_structure(&csrc);
    eris_codesrc_parse_file(&csrc);
    eris_codesrc_generate(&csrc);

    eris_const_function_t *entry = eris_load(&mod);

    eris_run(&mod, entry);

    eris_codesrc_destruct(&csrc);

    res = 0;

done:
    return res;
}
