#ifndef ERIS_LEXER_H
#define ERIS_LEXER_H

#include "ctk/lexer.h"
#include "ctk/token-list.h"
#include "ctk/text-source.h"

void eris_lex(ctk_textsrc_t *ts, ctk_tokenlist_t *toks);

#endif
