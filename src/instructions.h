#ifndef ERIS_INSTRUCTIONS_H
#define ERIS_INSTRUCTIONS_H

#define ERIS_INSTRUCTIONS_OTHER(X) \
        X(NOP, nop) \
        X(INVOKE, invoke) \
        X(IRETURN, ireturn)

#define ERIS_INSTRUCTIONS(X) \
        ERIS_INSTRUCTIONS_OTHER(X)

#endif
