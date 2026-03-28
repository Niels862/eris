#include "compiler/analyze.h"
#include "util/arena.h"
#include "util/list.h"
#include "util/error.h"

typedef struct {
    er_arena_t *scratch;
    er_buildfunc_t *bfunc;

    struct {
        er_irblock_t **data;
        size_t size;
        size_t cap;
    } tasks;
} er_anactx_t;

static void er_anactx_init(er_anactx_t *a, er_buildfunc_t *bfunc) {
    a->scratch = er_arena_new(256);
    a->bfunc = bfunc;

    ER_LIST_INIT(&a->tasks, 8);
}

static void er_anactx_destruct(er_anactx_t *a) {
    er_arena_delete(a->scratch);
}

static void er_add_task(er_anactx_t *a, er_irblock_t *task) {
    ER_LIST_ADD(&a->tasks, task);
}

static er_irblock_t *er_pop_task(er_anactx_t *a) {
    return a->tasks.data[--a->tasks.size];
}

typedef void(*er_ana_transfer_t)(er_irblock_t *, void *);
typedef bool(*er_ana_merge_t)(er_irblock_t *, void *);

static void er_analyze_dataflow(er_anactx_t *a, void *state, 
                                er_ana_transfer_t transfer, 
                                er_ana_merge_t merge) {
    ER_UNUSED(merge);
    
    er_add_task(a, a->bfunc->entry);

    while (a->tasks.size > 0) {
        er_irblock_t *task = er_pop_task(a);
        transfer(task, state);
    }
}

void er_transfer_stack_size(er_irblock_t *block, void *state) {
    ER_UNUSED(block);

    size_t size = 0;
    *(size_t *)state = size;
}

bool er_analyze_func(er_buildmod_t *bmod, er_buildfunc_t *bfunc) {
    ER_UNUSED(bmod);

    er_anactx_t a;
    er_anactx_init(&a, bfunc);

    size_t size;
    er_analyze_dataflow(&a, &size, er_transfer_stack_size, NULL);

    er_anactx_destruct(&a);

    return true;
}

bool er_analyze(er_buildmod_t *bmod) {
    bool succes = true;

    for (size_t i = 0; i < bmod->n_bfuncs; i++) {
        succes &= er_analyze_func(bmod, &bmod->bfuncs[i]);
    }

    return succes;
}
