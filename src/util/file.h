#ifndef ERIS_UTIL_FILE_H
#define ERIS_UTIL_FILE_H

#include <stddef.h>
#include <stdbool.h>

bool er_read_text_file(char const *filename, char **text, size_t *size);

#endif
