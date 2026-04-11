#include "compiler/analyze.h"
#include "compiler/transfer.h"
#include "util/arena.h"
#include "util/list.h"
#include "util/error.h"
#include <stdlib.h>
#include <assert.h>

typedef struct {
    er_irblock_t **data;
    size_t size;
    size_t cap;
} er_taskstack_t;

typedef void(*er_ana_transfer_t)(er_irblock_t *, void *, void *);

static void er_analyze_dataflow(er_irblock_t *entry,
                                void *state, void *ctx,
                                er_ana_transfer_t transfer) {    
    er_taskstack_t tasks;
    ER_LIST_INIT(&tasks, 16);
    
    ER_LIST_ADD(&tasks, entry);

    while (tasks.size > 0) {
        er_irblock_t *task = ER_LIST_POP(&tasks);
        transfer(task, state, ctx);
    }

    free(tasks.data);
}

static void er_transfer_semantics(er_irblock_t *block, 
                                  void *gstate, void *gctx) {
    for (size_t i = 0; i < block->n_nodes; i++) {
        er_transfer(&block->nodes[i], gstate, gctx);
    }
}

static void er_analyze_semantics(er_semctx_t *ctx) {
    er_semstate_t state;
    er_semstate_init(&state);

    er_analyze_dataflow(ctx->func->entry, &state, ctx, er_transfer_semantics);

    er_semstate_destruct(&state);
}

static bool er_analyze_func(er_buildctx_t *bctx, er_buildfunc_t *bfunc) {
    er_semctx_t sctx;
    er_semctx_init(&sctx, bctx, bfunc);

    er_analyze_semantics(&sctx);

    return true;
}

bool er_analyze(er_buildctx_t *bctx, er_buildmod_t *bmod) {
    bool s = true;

    for (size_t i = 0; i < bmod->n_bfuncs; i++) {
        s &= er_analyze_func(bctx, &bmod->bfuncs[i]);
    }

    return s;
}
