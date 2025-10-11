#include "ctk/string-span.h"
#include <stdio.h>

int main() {
    ctk_strspan_t span;
    ctk_strspan_init_from_string(&span, "hi :)");

    ctk_strspan_write_repr(&span, stdout);
    printf("\n");

    return 0;
}
