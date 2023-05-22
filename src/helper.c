#include <stdlib.h>
#include <string.h>

#include "helper.h"

/* helper function for cloning strings, akin to "strcpy" */
char* helper_clone_string(const char* original) {

    size_t length;
    char* clone;

    if (original == NULL) {

        return NULL;

    }

    length = strlen(original) + 1;
    clone = malloc(length);
    strcpy(clone, original);

    return clone;

}

static const char* digits_binary = "./";

/* helper function for writing the binary representation of the code */
void helper_encode_binary(int value, char* binary) {

    int index;
    char digit;

    binary[LIMIT_BINARY] = '\0';

    index = LIMIT_BINARY;
    while (index > 0) {

        index = index - 1;
        digit = digits_binary[(value & 1)];
        binary[index] = digit;

        value = value >> 1;

    }

}

