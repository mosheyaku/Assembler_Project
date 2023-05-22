#pragma once

#include "parts.h"
#include "line.h"
#include "instruction.h"

typedef void (*translator_error)(
    void* context,
    int code
);

typedef void (*translator_line)(
    void* context,
    line_t* line
);

/* structure for a translator object, holds parts of a line */
typedef struct translator {
    parts_t* parts;
} translator_t;

translator_t* translator_create();

void translator_destroy(translator_t* translator);

void translator_process(
    translator_t* translator,
    const char* line,
    void* context,
    translator_error callback_error,
    translator_line callback_line
);
