#include "util/file.h"
#include "util/alloc.h"
#include <stdio.h>
#include <stddef.h>

bool er_read_text_file(char const *filename, char **text, size_t *size) {
    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        return false;
    }
    
    size_t filesize = 0;
    size_t readsize = 256;
    char *buf = er_xmalloc(readsize);

    size_t nread;
    while ((nread = fread(buf + filesize, 1, readsize, file)) > 0) {
        filesize += nread;
        readsize *= 2;
        buf = er_xrealloc(buf, filesize + readsize);
    }

    *text = buf;
    *size = filesize;
 
    fclose(file);
  
    return true;
}
