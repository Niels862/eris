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

static er_type_t *er_peek(er_semstate_t *state, size_t at) {
    assert(state->size > at);
    return state->data[state->size - at - 1].type;
}

#define ER_CHECK_HANDLER(n) \
    static inline bool er_check_##n( \
        er_irnode_t *node, er_semstate_t *state, er_semctx_t *ctx)

#define ER_UNIMPLEMENTED_CHECK_HANDLER(n) \
    ER_CHECK_HANDLER(n) { \
        ER_UNUSED(node), ER_UNUSED(state), ER_UNUSED(ctx); \
        ER_UNIMPLEMENTED_FUNCTION(); \
        return true; \
    }

ER_CHECK_HANDLER(PUSHINT) {
    er_transfer(node, state, ctx);
    return true;
}

ER_CHECK_HANDLER(BINOP) {
    er_type_t *L = er_peek(state, 0);
    er_type_t *R = er_peek(state, 1);

    er_transfer(node, state, ctx);

    er_type_t *O = er_peek(state, 0);

    if (O == ctx->Error) {
        ER_FATAL("ERR");
    }

    return true;
}

ER_CHECK_HANDLER(RET) {
    er_transfer(node, state, ctx);
    return true;
}

static bool er_check_semantics(er_semctx_t *sctx) {
    bool s = true;

    er_semstate_t state;
    er_semstate_init(&state);
    
    for (size_t i = 0; i < sctx->func->n_blocks; i++) {
        er_irblock_t *block = &sctx->func->blocks[i];

        for (size_t j = 0; j < block->n_nodes; j++) {
            er_irnode_t *node = &block->nodes[j];

            switch (node->tag) {
                #define X(n, f) \
                    case ER_IR_##n: \
                        s &= er_check_##n(node, &state, sctx); \
                        break;

                ER_IRTAGS(X);

                #undef X
                
                default:
                    ER_UNHANDLED_SWITCH_VALUE("%d", node->tag);

            }
        }
    }

    er_semstate_destruct(&state);

    return s;
}

static bool er_analyze_func(er_buildctx_t *bctx, er_buildfunc_t *bfunc) {
    er_semctx_t sctx;
    er_semctx_init(&sctx, bctx, bfunc);

    er_analyze_semantics(&sctx);

    return er_check_semantics(&sctx);
}

bool er_analyze(er_buildctx_t *bctx, er_buildmod_t *bmod) {
    bool s = true;

    for (size_t i = 0; i < bmod->n_bfuncs; i++) {
        s &= er_analyze_func(bctx, &bmod->bfuncs[i]);
    }

    return s;
}
