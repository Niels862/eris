#ifndef ER_UTIL_ERROR_H
#define ER_UTIL_ERROR_H

#ifndef NDEBUG

#include <stdio.h>
#include <stdarg.h>

__attribute__((format(printf, 1, 2)))
static inline void eprintf(char const *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}

#endif // !NDEBUG

#define ER_UNUSED(v) (void)v

#endif
