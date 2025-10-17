#ifndef ERIS_SOURCE_H
#define ERIS_SOURCE_H

#include "ast.h"
#include "ctk/text-source.h"
#include "ctk/token-list.h"

typedef struct {
    ctk_textsrc_t textsrc;
    ctk_tokenlist_t toks;
    eris_node_source_t *root;
} eris_src_t;

void eris_src_init(eris_src_t *src, ctk_zstr_t filename, FILE *file);

void eris_src_destruct(eris_src_t *src);

void eris_src_lex(eris_src_t *src);

void eris_src_parse_file(eris_src_t *src);

#endif
