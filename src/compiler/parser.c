#include "compiler/parser.h"
#include "compiler/logger.h"
#include "util/alloc.h"
#include "util/error.h"
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>

static er_astdata_t const er_dummy_astdata;

typedef struct {
    er_buildmod_t *bmod;
    er_tok_t *toks;
    er_tok_t *curr;
} er_parsectx_t;

typedef struct {
    er_astnode_t **nodes;
    size_t size;
    size_t cap;
} er_nodelist_t;

er_astnode_t *er_astnode_alloc(er_buildmod_t *bmod, 
                               er_astkind_t kind, er_textpos_t pos, 
                               size_t datasize) {
    size_t size = sizeof(er_astnode_t) + datasize;
    er_astnode_t *n = er_pool_alloc(bmod->pool, size);

    n->kind = kind;
    n->pos = pos;
    memset(n + sizeof(er_astnode_t), 0, datasize);

    return n;
}

#define ER_AST_ALLOC(kind, pos, data) \
        er_astnode_alloc(p->bmod, kind, pos, sizeof(er_dummy_astdata.data))

static void er_nodelist_init(er_parsectx_t *p, er_nodelist_t *nl) {
    ER_UNUSED(p);

    nl->nodes = NULL;
    nl->size = 0;
    nl->cap = 0;
}

static void er_nodelist_add(er_parsectx_t *p, er_nodelist_t *nl, 
                            er_astnode_t *n) {
    ER_UNUSED(p);

    assert(n != NULL);

    if (nl->cap == 0) {
        nl->cap = 4;
        nl->nodes = er_xmalloc(nl->cap * sizeof(er_astnode_t *));
    } else {
        if (nl->size + 1 > nl->cap) {
            nl->cap *= 2;
            nl->nodes = er_xrealloc(nl->nodes, nl->cap * sizeof(er_astnode_t));
        }
    }

    nl->nodes[nl->size++] = n;
}

static void er_nodelist_move(er_parsectx_t *p, er_nodelist_t *nl, 
                             er_astnode_t ***dst, size_t *dst_size) {
    assert(*dst == NULL);
    assert(*dst_size == 0);

    size_t size = nl->size * sizeof(er_astnode_t *);
    
    if (nl->size == 0) {
        *dst = NULL;
    } else {
        *dst = er_pool_alloc(p->bmod->pool, size);
        memcpy(dst, nl->nodes, size);

        free(nl->nodes);
        nl->nodes = NULL;
    }

    *dst_size = nl->size;
}

static er_tok_t *er_consume(er_parsectx_t *p) {
    er_tok_t *tok = p->curr;
    if (tok->kind != ER_TOK_ENDOFINPUT) {
        p->curr++;
    }
    return tok;
}

static er_tok_t *er_accept(er_parsectx_t *p, er_tokkind_t kind) {
    if (p->curr->kind == kind) {
        return er_consume(p);
    }
    return NULL;
}

static er_tok_t *er_expect(er_parsectx_t *p, er_tokkind_t kind) {
    if (p->curr->kind == kind) {
        return er_consume(p);
    }
    er_err(p->bmod, p->curr->pos, 
           "expected %s, but got %s", 
           er_tokkind_name(kind), er_tokkind_name(p->curr->kind));
    return NULL;
}

static er_astnode_t *er_parse_func(er_parsectx_t *p) {
    er_textpos_t pos = p->curr->pos;

    if (!er_expect(p, ER_TOK_IDENTIFIER)) {
        return NULL;
    }

    er_nodelist_t stmts;
    er_nodelist_init(p, &stmts);

    er_astnode_t *n = ER_AST_ALLOC(ER_AST_FUNC, pos, func);
    er_nodelist_move(p, &stmts, &n->data.func.stmts, &n->data.func.n_stmts);

    return n;
}

static er_astnode_t *er_parse_mod(er_parsectx_t *p) {
    er_textpos_t pos = p->curr->pos;

    er_nodelist_t funcs;
    er_nodelist_init(p, &funcs);
    
    while (p->curr->kind != ER_TOK_ENDOFINPUT) {
        er_astnode_t *fn = er_parse_func(p);
        if (fn != NULL) {
            er_nodelist_add(p, &funcs, fn);
        } else {
            er_consume(p);
        }
    }

    er_astnode_t *n = ER_AST_ALLOC(ER_AST_MOD, pos, mod);
    er_nodelist_move(p, &funcs, &n->data.mod.funcs, &n->data.mod.n_funcs);

    return n;
}

er_astnode_t *er_parse(er_buildmod_t *bmod, er_tok_t *toks) {
    er_parsectx_t p = {
        .bmod = bmod,
        .toks = toks,
        .curr = toks,
    };

    er_astnode_t *mod = er_parse_mod(&p);

    return mod;
}
