#include "source.h"
#include "search.h"
#include "lexer.h"
#include "token.h"
#include "ast.h"
#include "ctk/text-source.h"
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

    eris_src_t src;
    eris_src_init(&src, filename, file);

    fclose(file);

    eris_src_lex(&src);
    eris_src_parse_file(&src);

    eris_src_destruct(&src);

    res = 0;

done:
    return res;
}
