#include "ctk/text-source.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
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

    ctk_textsrc_t ts;
    ctk_textsrc_init_file(&ts, filename, file);

    ctk_textsrc_write(&ts, stdout);

    ctk_textsrc_destruct(&ts);

    fclose(file);

    res = 0;

done:
    return res;
}
