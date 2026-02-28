#include "compiler/parser.h"
#include "compiler/logger.h"
#include "util/alloc.h"
#include "util/error.h"
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
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
    memset(&n->d, 0, datasize);

    return n;
}

#define ER_AST_ALLOC(p, kind, pos, data) \
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
        memcpy(*dst, nl->nodes, size);
        
        er_invalidate(nl->nodes, nl->size * sizeof(er_astnode_t *));
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

static er_astnode_t *er_parse_stmt(er_parsectx_t *p);

static bool er_parse_compound(er_parsectx_t *p, er_nodelist_t *nl) {
    er_nodelist_init(p, nl);

    if (er_expect(p, ER_TOK_LBRACE) == NULL) {
        return false;
    }

    bool err = false;
    while (p->curr->kind != ER_TOK_ENDOFINPUT 
            && p->curr->kind != ER_TOK_RBRACE) {
        er_astnode_t *n = er_parse_stmt(p);

        if (n != NULL) {
            er_nodelist_add(p, nl, n);
        } else {
            err = true;
        }
    }

    if (er_expect(p, ER_TOK_RBRACE) == NULL) {
        return false;
    }

    return err == false;
}

static void er_panic_stmt(er_parsectx_t *p) {
    do {
        if (p->curr->kind == ER_TOK_SEMICOLON) {
            er_consume(p);
            return;
        }

        if (p->curr->kind == ER_TOK_LBRACE) {
            er_nodelist_t dummy;
            er_parse_compound(p, &dummy);
            return;
        }

        if (p->curr->kind == ER_TOK_RBRACE) {
            return;
        }

        er_consume(p);
    } while (p->curr->kind != ER_TOK_ENDOFINPUT);
}

static er_astnode_t *er_parse_value(er_parsectx_t *p) {
    er_tok_t *tok = er_expect(p, ER_TOK_NUMBER);
    if (tok == NULL) {
        return NULL;
    }

    er_textpos_t pos = tok->pos;
    int64_t val = 0;

    er_astnode_t *n = ER_AST_ALLOC(p, ER_AST_INT, pos, e_int);
    n->d.e_int.val = val;

    return n;
}

static er_astnode_t *er_parse_expr(er_parsectx_t *p) {
    return er_parse_value(p);
}

static er_astnode_t *er_parse_return_stmt(er_parsectx_t *p) {
    er_tok_t *tok = er_expect(p, ER_TOK_RETURN);
    if (tok == NULL) {
        return NULL;
    }

    er_textpos_t pos = tok->pos;

    er_astnode_t *vn = NULL;
    if (p->curr->kind != ER_TOK_SEMICOLON) {
        vn = er_parse_expr(p);

        if (vn == NULL) {
            return NULL;
        }
    }

    if (er_expect(p, ER_TOK_SEMICOLON) == NULL) {
        return NULL;
    }

    er_astnode_t *n = ER_AST_ALLOC(p, ER_AST_RET, pos, s_ret);
    n->d.s_ret.val = vn;

    return n;
}

static er_astnode_t *er_parse_stmt(er_parsectx_t *p) {
    er_astnode_t *n = NULL;
    
    switch (p->curr->kind) {
        case ER_TOK_RETURN:
            n = er_parse_return_stmt(p);
            break;

        default:
            er_err(p->bmod, p->curr->pos, 
                   "expected statement, but got %s",
                  er_tokkind_name(p->curr->kind));
            break;
    }
    
    if (n == NULL) {
        er_panic_stmt(p);
    }

    return n;
}

static er_astnode_t *er_parse_func(er_parsectx_t *p) {
    if (er_expect(p, ER_TOK_IDENTIFIER) == NULL) {
        return NULL;
    }

    er_tok_t *nametok = er_expect(p, ER_TOK_IDENTIFIER);
    if (nametok == NULL) {
        return NULL;
    }

    er_textpos_t pos = nametok->pos;
    er_str_t *name = &nametok->text;
    
    er_expect(p, ER_TOK_LPAREN);
    er_expect(p, ER_TOK_RPAREN);

    er_nodelist_t stmts;
    if (!er_parse_compound(p, &stmts)) {
        return NULL;
    }

    er_astnode_t *n = ER_AST_ALLOC(p, ER_AST_FUNC, pos, func);
    n->d.func.name = *name;
    er_nodelist_move(p, &stmts, 
                     &n->d.func.stmts, 
                     &n->d.func.n_stmts);

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

    er_astnode_t *n = ER_AST_ALLOC(p, ER_AST_MOD, pos, mod);
    er_nodelist_move(p, &funcs, 
                     &n->d.mod.funcs, 
                     &n->d.mod.n_funcs);

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
