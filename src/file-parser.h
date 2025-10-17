#ifndef ERIS_FILE_PARSER_H
#define ERIS_FILE_PARSER_H

#include "ast.h"
#include "ctk/span.h"

eris_node_source_t *eris_parse_file(ctk_span_t *span);

#endif
