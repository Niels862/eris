#ifndef ERIS_SOURCE_H
#define ERIS_SOURCE_H

#include "ctk/text-source.h"
#include "ctk/token-list.h"

typedef struct {
    ctk_zstr_t filename;
    ctk_textsrc_t textsrc;
    ctk_tokenlist_t toks;
} eris_source_t;

#endif
