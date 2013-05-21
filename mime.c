#include <stdlib.h>
#include <string.h>
#include "mime.h"
#include "mime.data.h"

static const char *_get_file_extension(const char *file_name) {
    const char *file_extension = "bin";
    const char *p;
    for (p = file_name; *p != 0; p++) {
        if (*p == '.') {
            file_extension = p+1;
        }
    }
    return file_extension;
}

static int _extcmp(const void *a, const void *b) {
    const char *a2 = (const char *)a;
    const char **b2 = (const char **)b;
    return strcmp(a2,*b2);
}

const char *get_mime_type_for_file_name(const char *file_name) {
    const char *file_extension;
    const char **r;
    if (!file_name) return "application/octet-stream";
    file_extension = _get_file_extension(file_name);
    r = bsearch(file_extension, mime_exts, MIME_NUM_EXTS, sizeof(const char *), _extcmp);
    if (!r) {
        return "application/octet-stream";
    } else {
        return mime_types[mime_inds[(r-mime_exts)]];
    }
}
