#ifndef ERIS_PARSER_H
#define ERIS_PARSER_H

#include "ctk/parser.h"

typedef ctk_parser_t eris_parser_t;

void eris_parser_init(eris_parser_t *parser, ctk_span_t *span);

#endif