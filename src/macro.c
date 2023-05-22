#include <stdlib.h>

#include "macro.h"
#include "line.h"
#include "helper.h"

#define CHUNK_SIZE (8)

/* function that creates a "macro_t" object */
macro_t* macro_create(const char* name) {

    macro_t* macro = malloc(sizeof(*macro));
    macro->name = helper_clone_string(name);

    macro->size_lines = 0;
    macro->limit_lines = CHUNK_SIZE;
    macro->list_lines = calloc(
        macro->limit_lines,
        sizeof(*macro->list_lines)
    );

    return macro;

}

/* function that releases the memory held by a "macro_t" object */
void macro_destroy(macro_t* macro) {

    size_t index;

    for (index = 0; index < macro->size_lines; index++) {

        line_destroy(macro->list_lines[index]);

    }

    free(macro->list_lines);
    free(macro->name);
    free(macro);

}
