#include <stdint.h>
#include <stddef.h>

typedef struct {
    char const *start;
    char const *end;
} er_strspan_t;

typedef struct {
    uint16_t line;
    uint16_t col;
} er_textpos_t;

typedef enum {
    // ...
} er_tokenkind_t;

typedef struct {
    er_tokenkind_t kind;
    er_textpos_t pos;
    er_strspan_t text;
} er_token_t;

typedef enum {
    // ...
} er_astkind_t;

typedef struct { // AST consists of pool-owned data structures
    er_astkind_t kind;
    er_textpos_t pos;

    union {
        struct {
            er_astnode_t *cond;
            er_astnode_t *body;
        } while_stmt;

        struct {
            er_astnode_t **stmts;
            size_t n_stmts;
        } compound;

        struct {
            er_strspan_t text;
        } id;

        // ...
    } data;
} er_astnode_t;

typedef enum {
    // ...
} er_symkind_t;

typedef struct {
    er_symkind_t kind;
    er_strspan_t text;

    union {
        typedef struct {
            er_symmap_t syms;
        } class_def;

        typedef struct {
            er_type_t *type;
        } var_def;
    } data;
} er_sym_t;

typedef struct {
    // opaque, only interface with functions
} er_symmap_t;

typedef enum {
    // ...
} er_typekind_t;

typedef struct {
    er_typekind_t kind;

    union {
        struct {
            er_sym_t *class_def;
        } named;

        struct {
            er_type_t *type;
        } nullable;
    } data;
} er_type_t;

typedef struct {
    // opaque, only interface with functions
} er_mempool_t;

typedef struct {
    er_astnode_t *ast;
    er_irblock_t *ir;
    er_mempool_t pool;
} er_modbuilder_t;

typedef enum {
    // ...
} er_irkind_t;

typedef struct {
    er_irkind_t kind;

    union {
        struct {
            er_sym_t *sym;
        } push_global;

        struct {
            er_sym_t *sym;
        } push_local;

        struct {
            int64_t lit;
        } push_int_lit;

        struct {
            er_irblock_t *next_if_true;
            er_irblock_t *next_if_false;
        } branch;

        struct {
            er_irblock_t *next;
        } jump;
    } data;
} er_irnode_t;

typedef struct {
    er_irstate_t state;
    er_irnode_t *nodes;
    size_t n_nodes;
} er_irblock_t;

// Used for forward flowing data analysis:
// - flow sensitive typing
// - flow sensitive nullability
typedef struct {
    er_type_t *type;
} er_irvalue_t;

typedef struct {
    er_irvalue_t *values; 
    // n_values = 1 + n_locals + n_stack. [0] = NULL -> never accessed
    size_t n_locals;
    size_t n_stack;
} er_irstate_t;

// Following is Module Target: Shared between Compile and Runtime

typedef struct {
    uint16_t me; // -> mod_ref

    er_const_t *consts;
    uint16_t n_consts;

    er_func_t *funcs;
    uint16_t n_funcs;
} er_mod_t;

typedef enum {
    // ...
} er_constkind_t;

typedef struct {
    er_constkind_t kind;

    // Will be different structs with shared header in implementation, to save 
    // memory and allow variable length for string
    union {
        struct {
            int64_t value;
        } int_lit;

        struct {
            uint16_t len;
            char s[];
        } str;

        struct {
            uint16_t name; // -> str
        } mod_ref;

        struct {
            uint16_t mod; // -> mod_ref
            uint16_t name; // -> str
        } func_ref;
    } data;
} er_const_t;

typedef struct {
    uint16_t n_params;
    uint16_t n_locals;
    uint16_t n_temps;

    uint16_t code_size;
    uint8_t code[];
} er_func_t;

// Following is exclusive to runtime

typedef union {
    er_mod_t *mod;
    er_func_t *func;
} er_const_ref_t;

typedef struct {
    er_mod_t mod;

    // Shadow table of consts, maps const index to resolved reference
    er_const_ref_t *refs;
} er_rtmod_t;
