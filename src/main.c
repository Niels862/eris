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

    ctk_textsrc_t ts;
    ctk_textsrc_init_file(&ts, filename, file);

    ctk_tokenlist_t toks;
    ctk_tokenlist_init(&toks);

    eris_lex(&ts, &toks);
    ctk_tokenlist_lock(&toks);

    for (size_t i = 0; i < toks.size; i++) {
        ctk_token_write(&toks.data[i], stdout);
        printf("\n");
    }

    printf("lexed %ld tokens.\n", toks.size);
    
    ctk_textsrc_destruct(&ts);

    fclose(file);

    eris_node_stmt_t *node = eris_node_return_new(NULL, eris_node_intlit_new(NULL, 42));

    ctk_rtti_write(node, 0, stdout);

    ctk_rtti_delete(node);

    res = 0;

done:
    return res;
}
