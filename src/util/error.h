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

#define ER_EMPTY char _

#define ER_FATAL(...) er_fatal(__FILE__, __LINE__, __VA_ARGS__)

#define ER_UNHANDLED_SWITCH_VALUE(format, value) \
        ER_FATAL("unhandled switch value: " format "\n", value)

__attribute__((noreturn))
void er_fatal(char const *file, int line, char const *format, ...);

#endif
