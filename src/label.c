#include <stdlib.h>

#include "label.h"
#include "helper.h"

/* function that creates a "label_t" object */
label_t* label_create(const char* name, int value) {

    label_t* label;

    label = malloc(sizeof(*label));
    label->name = helper_clone_string(name);
    label->full_address = value;
    label->entry = false;

    return label;

}

/* function that releases the memory held by a "label_t" object */
void label_destroy(label_t* label) {

    free(label->name);
    free(label);

}