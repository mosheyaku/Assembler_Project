#pragma once

#include <stdlib.h>

/* structure that represents the many parts of a line */
typedef struct parts {
    size_t* list_indexes;
    size_t size_indexes;
    size_t limit_indexes;

    char* parts_memory;
    size_t parts_size;
    size_t parts_limit;
} parts_t;

parts_t* parts_create();

void parts_destroy(parts_t* parts);

/* breaks line into parts */
void parts_process(parts_t* parts, const char* line);

/* gets the chunk at the given index */
char* parts_get(parts_t* parts, size_t index);
