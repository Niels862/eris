#include "eris.h"

ctk_textctx_style_t const eris_context_style = {
    .useansi    = true,
    .usemarker  = true,
    
    .markstart  = '^',
    .markinter  = '~',
    .markend    = '~',

    .linepad    = 5,

    .focus      = CTK_ANSI_FG_BRIGHT(CTK_ANSI_RED),
    .marker     = CTK_ANSI_FG_BRIGHT(CTK_ANSI_YELLOW),
};
