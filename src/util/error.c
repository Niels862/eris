#include "error.h"
#include <stdlib.h>

void er_fatal(char const *file, int line, char const *format, ...) {
    fprintf(stderr, "Fatal error at %s:%d: ", file, line);

    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    fprintf(stderr, "\n");

    exit(1);
}
