#pragma once

#include <stdlib.h>

#include "line.h"

/* structure that represents a processed macro */
typedef struct macro {
    char* name;

    line_t** list_lines;
    size_t size_lines;
    size_t limit_lines;
} macro_t;

macro_t* macro_create(const char* name);

void macro_destroy(macro_t* macro);

void macro_add(macro_t* macro, line_t* line);
