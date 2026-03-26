#ifndef ER_LIST_H
#define ER_LIST_H

#include "util/alloc.h"

#define ER_LIST_INIT(list, init_size) \
    do { \
        (list)->cap = (init_size); \
        (list)->size = 0; \
        (list)->data = er_xmalloc((init_size) * sizeof(*((list)->data))); \
    } while (0)

#define ER_LIST_RESERVE_N(list, add_n) \
    do { \
        if ((list)->size + add_n > (list)->cap) { \
            do { (list)->cap *= 2; } while ((list)->size + add_n < (list)->cap); \
            (list)->data = er_xrealloc((list)->data, (list)->cap); \
        } \
    } while (0)

#define ER_LIST_RESERVE_ONE(list) \
    do { \
        if ((list)->size + 1 > (list)->cap) { \
            (list)->cap *= 2; \
            (list)->data = er_xrealloc((list)->data, (list)->cap); \
        } \
    } while (0)

#endif
