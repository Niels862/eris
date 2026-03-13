#include "codegen.h"
#include "util/error.h"
#include "util/alloc.h"
#include <string.h>

void er_codegen_init(er_buildmod_t *bmod) {
    bmod->mod = er_xmalloc(sizeof(er_mod_t));
    memset(bmod->mod, 0, sizeof(er_mod_t));
}
