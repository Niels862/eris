#include "compiler/analyze.h"
#include "util/arena.h"
#include "util/list.h"
#include "util/error.h"
#include <stdlib.h>
#include <assert.h>

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

typedef struct {
    er_type_t **data;
    size_t size;
    size_t cap;

    er_buildctx_t *bctx;
    er_buildfunc_t *bfunc;

    er_type_t *rettype;
} er_semstate_t;

static void er_semstate_init(er_semstate_t *state,  er_buildctx_t *bctx, 
                             er_buildfunc_t *bfunc) {
    ER_LIST_INIT(state, 16);

    state->bctx = bctx;
    state->bfunc = bfunc;
}

static void er_semstate_destruct(er_semstate_t *state) {
    free(state->data);
}

static void er_transfer_node_semantics(er_irnode_t *node, 
                                       er_semstate_t *state) {    
    switch (node->tag) {
        case ER_IR_PUSHINT: {
            ER_LIST_ADD(state, state->bctx->Int);
            break;
        }

        case ER_IR_BINOP: {
            er_type_t *L = ER_LIST_POP(state);
            er_type_t *R = ER_LIST_POP(state);

            if (L == state->bctx->Int && R == state->bctx->Int) {
                ER_LIST_ADD(state, state->bctx->Int);
            } else {
                ER_FATAL("TODO");
            }

            break;
        }

        case ER_IR_RET: {
            er_type_t *V = ER_LIST_POP(state);

            if (V != state->bfunc->type->rettype) {
                ER_FATAL("TODO");
            }

            break;
        }

        default:
            ER_UNHANDLED_SWITCH_VALUE("%d", node->tag);
    }
}

static void er_transfer_semantics(er_irblock_t *block, void *gstate) {
    er_semstate_t *state = gstate;

    for (size_t i = 0; i < block->n_nodes; i++) {
        er_transfer_node_semantics(&block->nodes[i], state);
    }
}

static bool er_merge_semantics(er_irblock_t *block, void *gstate) {
    ER_UNUSED(block), ER_UNUSED(gstate);
    return true;
}

static void er_analyze_semantics(er_anactx_t *a, er_buildctx_t *bctx, 
                                 er_buildfunc_t *bfunc) {
    er_semstate_t state; 
    er_semstate_init(&state, bctx, bfunc);

    er_analyze_dataflow(a, &state, er_transfer_semantics, er_merge_semantics);

    er_semstate_destruct(&state);
}

static bool er_analyze_func(er_buildctx_t *bctx, er_buildfunc_t *bfunc) {
    er_anactx_t a;
    er_anactx_init(&a, bfunc);

    er_analyze_semantics(&a, bctx, bfunc);
    
    er_anactx_destruct(&a);

    return true;
}

bool er_analyze(er_buildctx_t *bctx, er_buildmod_t *bmod) {
    bool s = true;

    for (size_t i = 0; i < bmod->n_bfuncs; i++) {
        s &= er_analyze_func(bctx, &bmod->bfuncs[i]);
    }

    return s;
}
