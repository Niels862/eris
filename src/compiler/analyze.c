#include "compiler/analyze.h"
#include "util/arena.h"
#include "util/list.h"
#include "util/error.h"
#include <stdlib.h>

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
    free(a->tasks.data);
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

#define ER_STACKSIZE_UNSET -1

typedef struct {
    size_t size;
    size_t max_size;
    size_t *in;
} er_stacksizestate_t;

static void er_stacksize_init(er_stacksizestate_t *state, 
                              er_buildfunc_t *bfunc) {
    state->size = 0;
    state->max_size = 0;
    state->in = er_xmalloc(bfunc->n_blocks * sizeof(size_t));

    for (size_t i = 0; i < bfunc->n_blocks; i++) {
        if (i == ER_IRBLOCKREF_ENTRY) {
            state->in[i] = 0;
        } else {
            state->in[i] = ER_STACKSIZE_UNSET;
        }
    }
}

static void er_stacksize_destruct(er_stacksizestate_t *state) {
    free(state->in);
}

static void er_stacksize_transfer(er_irblock_t *block, void *gstate) {
    er_stacksizestate_t *state = gstate;

    state->size = state->in[block->ref];

    for (size_t i = 0; i < block->n_nodes; i++) {
        er_irnode_t *node = &block->nodes[i];

        switch (node->tag) {
            case ER_IR_PUSHINT:
                state->size += 1;
                break;

            case ER_IR_BINOP:
                state->size -= 1;
                break;

            case ER_IR_RET:
                state->size = 0;
                break;

            default:
                ER_UNHANDLED_SWITCH_VALUE("%d", node->tag);
        }

        if (state->size > state->max_size) {
            state->max_size = state->size;
        }
    }
}

static void er_stacksize_analyze(er_anactx_t *a) {
    er_stacksizestate_t state;
    er_stacksize_init(&state, a->bfunc);

    er_analyze_dataflow(a, &state, er_stacksize_transfer, NULL);
    a->bfunc->n_temps = state.max_size;

    er_stacksize_destruct(&state);
}

static bool er_analyze_func(er_buildmod_t *bmod, er_buildfunc_t *bfunc) {
    ER_UNUSED(bmod);

    er_anactx_t a;
    er_anactx_init(&a, bfunc);

    er_stacksize_analyze(&a);

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
