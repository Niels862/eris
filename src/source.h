#ifndef ERIS_SOURCE_H
#define ERIS_SOURCE_H

#include "ast.h"
#include "module.h"
#include "ctk/text-source.h"
#include "ctk/token-list.h"

typedef struct {
    ctk_textsrc_t textsrc;
    ctk_tokenlist_t toks;
    eris_node_source_t *root;
    eris_module_t *mod;
} eris_codesrc_t;

void eris_codesrc_init(eris_codesrc_t *csrc, ctk_zstr_t filename, FILE *file,
                       eris_module_t *mod);

void eris_codesrc_destruct(eris_codesrc_t *csrc);

void eris_codesrc_lex(eris_codesrc_t *csrc);

void eris_codesrc_parse_file(eris_codesrc_t *csrc);

void eris_codesrc_generate(eris_codesrc_t *csrc);

#endif
