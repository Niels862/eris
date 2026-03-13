#include "compiler/build.h"
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "usage: %s [entry]\n", argv[0]);
        return 1;
    }

    er_mod_t **mods = er_build(argv[1]);
    if (mods == NULL) {
        return 1;
    }

    for (size_t i = 0; mods[i] != NULL; i++) {
        er_mod_print(mods[i]);
        free(mods[i]);
    }
    free(mods);

    return 0;
}
