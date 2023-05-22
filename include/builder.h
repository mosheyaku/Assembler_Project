#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "booleans.h"
#include "label.h"
#include "macro.h"
#include "line.h"

/* structure that represents an object used in the build process */
typedef struct builder {
    line_t** list_lines;
    size_t size_lines;
    size_t limit_lines;

    label_t** list_labels;
    size_t size_labels;
    size_t limit_labels;

    macro_t** list_macros;
    size_t size_macros;
    size_t limit_macros;

    macro_t* current_macro;

    FILE* file_as;
    FILE* file_am;
    FILE* file_ob;
    FILE* file_ent;
    FILE* file_ext;

    size_t length_code;
    size_t length_data;

    size_t length_entry;
    size_t length_extern;

    char* current_line;
    size_t errors_found;
    size_t line_number;
    size_t program_counter;
    bool inside_macro;
} builder_t;

builder_t* builder_create(
    FILE* file_as,
    FILE* file_am,
    FILE* file_ob
);

void builder_destroy(builder_t* builder);

void builder_first_pass(builder_t* builder);

void builder_second_pass(
    builder_t* builder,
    FILE* file_ent,
    FILE* file_ext
);
