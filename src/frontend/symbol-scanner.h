#ifndef ERIS_SYMBOL_SCANNER_H
#define ERIS_SYMBOL_SCANNER_H

#include "frontend/symbol-table.h"
#include "ctk/span.h"

void eris_scan_symbols(ctk_span_t *toks, eris_scopelist_t *scopes);

#endif
