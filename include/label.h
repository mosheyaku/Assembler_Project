#pragma once

#include "booleans.h"

/* structure that represents labels used in assembler source code files */
typedef struct label {
    char* name;
    int full_address;
    bool entry;
} label_t;

label_t* label_create(const char* name, int value);

void label_destroy(label_t* label);
