#include "compiler/parse.h"
#include "compiler/logger.h"
#include "util/alloc.h"
#include "util/error.h"
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <assert.h>
#include <errno.h>

static er_astdata_t const er_dummy_astdata;

typedef struct {
    er_buildmod_t *bmod;
    er_arena_t *scratch;
    er_tok_t *toks;
    er_tok_t *curr;
    bool error;
} er_parsectx_t;

#define ER_NODELIST_STATIC_DATA_SIZE 16

typedef struct {
    er_astnode_t **nodes;
    size_t size;
    size_t cap;
    er_astnode_t *data[ER_NODELIST_STATIC_DATA_SIZE];
} er_nodelist_t;

static void er_parsectx_init(er_parsectx_t *p, er_buildmod_t *bmod) {
    p->bmod = bmod;
    p->toks = p->curr = bmod->toks;
    p->scratch = er_arena_new(256);
    p->error = false;
}

static void er_parsectx_destruct(er_parsectx_t *p) {
    er_arena_delete(p->scratch);
}

er_astnode_t *er_astnode_alloc(er_parsectx_t *p, 
                               er_astkind_t kind, er_textpos_t pos, 
                               size_t datasize) {
    size_t size = offsetof(er_astnode_t, data) + datasize;
    er_astnode_t *n = er_arena_alloc(p->bmod->arenas.parse, size);

    n->kind = kind;
    n->pos = pos;
    memset(&n->data, 0, datasize);

    return n;
}

#define ER_AST_ALLOC(p, kind, pos, data) \
        er_astnode_alloc(p, kind, pos, sizeof(er_dummy_astdata.data))

static void er_nodelist_init(er_parsectx_t *p, er_nodelist_t *nl) {
    ER_UNUSED(p);

    nl->nodes = nl->data;
    nl->size = 0;
    nl->cap = ER_NODELIST_STATIC_DATA_SIZE;
}

static void er_nodelist_add(er_parsectx_t *p, er_nodelist_t *nl, 
                            er_astnode_t *n) {
    assert(n != NULL);
    
    if (nl->size + 1 > nl->cap) {
        nl->nodes = er_arena_realloc(p->scratch, nl->nodes, 
                                     nl->cap, 2 * nl->cap, 
                                     sizeof(er_astnode_t *));
        nl->cap *= 2;
    }

    nl->nodes[nl->size] = n;
    nl->size++;
}

static void er_nodelist_move(er_parsectx_t *p, er_nodelist_t *nl, 
                             er_astnode_t ***dst, size_t *dst_size) {
    assert(*dst == NULL);
    assert(*dst_size == 0);
    
    if (nl->size == 0) {
        *dst = NULL;
    } else {
        *dst = er_arena_realloc(p->bmod->arenas.parse, 
                                nl->nodes, nl->size, nl->size, 
                                sizeof(er_astnode_t *));
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

static er_tok_t *er_peek(er_parsectx_t *p, size_t n) {
    er_tok_t *tok = p->curr;

    for (size_t i = 0; i < n; i++) {
        if (tok->kind == ER_TOK_ENDOFINPUT) {
            return tok;
        }
        tok++;
    }

    return tok;
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

static bool er_is_typename(er_parsectx_t *p, er_tok_t *name, bool error) {
    er_sym_t *sym = er_symtab_lookup(&p->bmod->globals, &name->text);
    if (sym == NULL) {
        if (error) {
            er_err(p->bmod, name->pos, "'%.*s' is not defined", 
                name->text.len, name->text.data);
        }
        return false;
    }

    if (sym->kind != ER_SYM_CLASS) {
        if (error) {
            er_err(p->bmod, name->pos, "'%.*s' is not a typename", 
                name->text.len, name->text.data);
        }
        return false;
    }

    return true;
}

static er_astnode_t *er_parse_annotation(er_parsectx_t *p) {
    er_tok_t *name = er_expect(p, ER_TOK_IDENTIFIER);
    if (name == NULL) {
        return NULL;
    }

    if (!er_is_typename(p, name, true)) {
        return NULL;
    }

    er_astnode_t *n = ER_AST_ALLOC(p, ER_AST_IDENT, name->pos, Ident);
    n->data.Ident.name = name->text;

    return n;
}

#define ER_INTEGER_BUF_STATIC_SIZE 64

static int er_parse_base_modifier(er_str_t *s) {
    if (s->len < 2) {
        return 10;
    }

    if (s->data[0] == '0') {
        if (s->data[1] == 'x' || s->data[1] == 'X') {
            er_str_ltrim(s, 2);
            return 16;
        }

        if (s->data[1] == 'b' || s->data[1] == 'B') {
            er_str_ltrim(s, 2);
            return 2;
        }
    }

    return 10;
}

static er_astnode_t *er_parse_integer(er_parsectx_t *p) {
    char static_buf[ER_INTEGER_BUF_STATIC_SIZE];

    char *buf = static_buf;
    size_t buf_size = 0;

    if (er_accept(p, ER_TOK_MINUS)) {
        buf[0] = '-';
        buf_size = 1;
    }

    er_tok_t *tok = er_expect(p, ER_TOK_INTEGER);
    if (tok == NULL) {
        return NULL;
    }

    er_str_t str = tok->text;

    int base = er_parse_base_modifier(&str);

    if (buf_size + tok->text.len + 1 > ER_INTEGER_BUF_STATIC_SIZE) {
        buf = er_arena_realloc(p->scratch, buf, buf_size, tok->text.len, 1);
    }

    bool leading_zero = false;

    for (int i = 0; i < str.len; i++) {
        char c = str.data[i];
        if (isalnum(c)) {
            if (c == '0' && i == 0) {
                leading_zero = true;
            }

            buf[buf_size] = c;
            buf_size++;
        } else if (c == '_') {
            if (i == str.len - 1) {
                er_err(p->bmod, tok->pos,
                       "trailing underscore in integer literal is disallowed");
                return NULL;
            }
        } else {
            ER_FATAL("unexpected character in integer literal");
        }
    }

    buf[buf_size] = '\0';

    char *end;
    int64_t val = strtoll(buf, &end, base);

    if (end != buf + buf_size) {
        er_err(p->bmod, tok->pos, "malformed integer literal");
        return NULL;
    }

    if (errno == ERANGE) {
        er_err(p->bmod, tok->pos, "integer literal out of range");
        return NULL;
    }

    if (val != 0 && leading_zero) {
        er_err(p->bmod, tok->pos, 
               "leading zero in integer literal is disallowed");
        return NULL;
    }

    er_astnode_t *n = ER_AST_ALLOC(p, ER_AST_INT, tok->pos, Int);
    n->data.Int.val = val;

    return n;
}

static er_astnode_t *er_parse_name(er_parsectx_t *p) {
    er_tok_t *name = er_expect(p, ER_TOK_IDENTIFIER);
    if (name == NULL) {
        return NULL;
    }

    er_astnode_t *n = ER_AST_ALLOC(p, ER_AST_IDENT, name->pos, Ident);
    n->data.Ident.name = name->text;

    return n;
}

static er_astnode_t *er_parse_value(er_parsectx_t *p) {
    if (p->curr->kind == ER_TOK_INTEGER) {
        return er_parse_integer(p);
    }

    if (p->curr->kind == ER_TOK_IDENTIFIER) {
        return er_parse_name(p);
    }

    return NULL;
}

typedef struct {
    er_tokkind_t kind;
    int op; /* unop or binop */
} er_opspec_t;

static er_tok_t *er_find_op(er_parsectx_t *p, er_opspec_t const *ops, 
                            size_t n_ops, int *op) {
    er_tokkind_t kind = p->curr->kind;
    
    for (size_t i = 0; i < n_ops; i++) {
        if (ops[i].kind == kind) {
            *op = ops[i].op;
            return er_consume(p);
        }
    }

    return NULL;
}

static er_astnode_t *er_parse_leftop(er_parsectx_t *p, 
                                     er_opspec_t const *ops, 
                                     size_t n_ops, 
                                     er_astnode_t *(*next)(er_parsectx_t *)) {
    er_astnode_t *left = next(p);
    if (left == NULL) {
        return NULL;
    }

    int op;
    er_tok_t *tok;
    while ((tok = er_find_op(p, ops, n_ops, &op)) != NULL) {
        er_astnode_t *right = next(p);
        if (right == NULL) {
            return NULL;
        }

        er_astnode_t *n = ER_AST_ALLOC(p, ER_AST_BINOP, tok->pos, BinOp);
        n->data.BinOp.op = op;
        n->data.BinOp.left = left;
        n->data.BinOp.right = right;

        left = n;
    }

    return left;
}

static er_astnode_t *er_parse_mul_expr(er_parsectx_t *p) {
    static const er_opspec_t ops[] = {
        { ER_TOK_ASTERISK,  ER_BINOP_MUL },
        { ER_TOK_SLASH,     ER_BINOP_DIV },
    };

    return er_parse_leftop(p, ops, 2, er_parse_value);
}

static er_astnode_t *er_parse_add_expr(er_parsectx_t *p) {
    static const er_opspec_t ops[] = {
        { ER_TOK_PLUS,      ER_BINOP_ADD },
        { ER_TOK_MINUS,     ER_BINOP_SUB },
    };

    return er_parse_leftop(p, ops, 2, er_parse_mul_expr);
}

static er_astnode_t *er_parse_expr(er_parsectx_t *p) {
    return er_parse_add_expr(p);
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

    er_astnode_t *n = ER_AST_ALLOC(p, ER_AST_RET, pos, Ret);
    n->data.Ret.val = vn;

    return n;
}

static er_astnode_t *er_parse_func(er_parsectx_t *p, 
                                   er_astnode_t *ret_anno, 
                                   er_tok_t *name) {    
    er_expect(p, ER_TOK_LPAREN);
    er_expect(p, ER_TOK_RPAREN);

    er_nodelist_t stmts;
    if (!er_parse_compound(p, &stmts)) {
        return NULL;
    }

    er_astnode_t *n = ER_AST_ALLOC(p, ER_AST_FUNC, name->pos, Func);
    n->data.Func.name = name->text;
    n->data.Func.ret_anno = ret_anno;
    er_nodelist_move(p, &stmts, 
                     &n->data.Func.stmts, 
                     &n->data.Func.n_stmts);

    return n;
}

static er_astnode_t *er_parse_var(er_parsectx_t *p, 
                                  er_astnode_t *anno, 
                                  er_tok_t *name) {
    er_astnode_t *value = NULL;

    if (er_accept(p, ER_TOK_EQ) != NULL) {
        value = er_parse_expr(p);
        if (value == NULL) {
            return NULL;
        }
    }

    if (er_expect(p, ER_TOK_SEMICOLON) == NULL) {
        return NULL;
    }

    er_astnode_t *n = ER_AST_ALLOC(p, ER_AST_VAR, name->pos, Var);
    n->data.Var.name = name->text;
    n->data.Var.anno = anno;
    n->data.Var.value = value;

    return n;
}

static er_astnode_t *er_parse_decl(er_parsectx_t *p) {
    er_astnode_t *anno = er_parse_annotation(p);
    if (anno == NULL) {
        return NULL;
    }

    er_tok_t *name = er_expect(p, ER_TOK_IDENTIFIER);
    if (name == NULL) {
        return NULL;
    }

    if (p->curr->kind == ER_TOK_LPAREN) {
        return er_parse_func(p, anno, name);
    } else {
        return er_parse_var(p, anno, name);
    }
}

static er_astnode_t *er_parse_stmt(er_parsectx_t *p) {
    er_astnode_t *n = NULL;
    
    switch (p->curr->kind) {
        case ER_TOK_RETURN:
            n = er_parse_return_stmt(p);
            break;

        case ER_TOK_IDENTIFIER: {
            if (er_is_typename(p, p->curr, false)) {
                n = er_parse_decl(p);
            } else {
                n = er_parse_expr(p);
            }
            break;
        }

        default:
            er_err(p->bmod, p->curr->pos, 
                   "expected statement, but got %s",
                  er_tokkind_name(p->curr->kind));
            break;
    }
    
    if (n == NULL) {
        p->error = true;
        er_panic_stmt(p);
    }

    return n;
}

static er_astnode_t *er_parse_mod(er_parsectx_t *p) {
    er_textpos_t pos = p->curr->pos;

    er_nodelist_t funcs;
    er_nodelist_init(p, &funcs);
    
    while (p->curr->kind != ER_TOK_ENDOFINPUT) {
        er_astnode_t *n = er_parse_stmt(p);
        if (n != NULL) {
            if (n->kind == ER_AST_FUNC) {
                er_nodelist_add(p, &funcs, n);
            } else {
                ER_NOT_IMPLEMENTED();
            }
        }
    }

    er_astnode_t *n = ER_AST_ALLOC(p, ER_AST_MOD, pos, Mod);
    er_nodelist_move(p, &funcs, 
                     &n->data.Mod.funcs, 
                     &n->data.Mod.n_funcs);

    return n;
}

bool er_parse(er_buildmod_t *bmod) {
    assert(bmod->toks != NULL);
    assert(bmod->root == NULL);

    er_parsectx_t p;
    er_parsectx_init(&p, bmod);

    bmod->root = er_parse_mod(&p);

    bool success = !p.error;
    er_parsectx_destruct(&p);

    return success;
}
