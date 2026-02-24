#include "compiler/build.h"
#include "util/file.h"
#include "util/alloc.h"
#include "util/error.h"
#include <stdio.h>
#include <string.h>

#define ER_PATH_BUFFER_SIZE 4096

static er_buildmod_t *er_buildmod_read_path(char const *path) {
    char *text;
    size_t size;
    if (!er_read_text_file(path, &text, &size)) {
        return NULL;
    }

    er_buildmod_t *bmod = er_xmalloc(sizeof(er_buildmod_t));

    bmod->pool = er_pool_new(4096);
    bmod->text = text;
    bmod->size = size;
    bmod->filename = er_pool_string_alloc(bmod->pool, path, -1);

    return bmod;
}

er_buildmod_t *er_buildmod_read(char const *module) {
    char const *paths[] = {
        "testdata/"
    };

    for (size_t i = 0; i < sizeof(paths) / sizeof(*paths); i++) {
        char buf[ER_PATH_BUFFER_SIZE];

        strcpy(buf, paths[i]);
        strcat(buf, module);
        strcat(buf, ".eris");

        if (er_is_file(buf)) {
            return er_buildmod_read_path(buf);
        }
    }

    return NULL;
}
