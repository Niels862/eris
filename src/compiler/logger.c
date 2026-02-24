#include "compiler/logger.h"
#include <stdio.h>
#include <stdarg.h>

void er_err(er_buildmod_t *bmod, er_textpos_t pos, char const *msg, ...) {
    va_list args;

    fprintf(stderr, "\033[91m" "[ERROR] %s:%d:%d: ", 
            bmod->filename, pos.line, pos.col);

    va_start(args, msg);
    vfprintf(stderr, msg, args);
    va_end(args);

    fprintf(stderr, "\033[0m" "\n");
}
