#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "booleans.h"
#include "parts.h"

#define MEMORY_SIZE (512)
#define STRIDE_SIZE (8)

/* structure for chopping line into parts */
typedef struct chopper {
    char* input_line;
    size_t index_iterator;
    size_t index_source;
    size_t index_target;
    size_t part_length;
    char character;
    char quotation;
} chopper_t;

static void chopper_flush(chopper_t* chopper, parts_t* parts);

static void chopper_process(parts_t* parts, const char* line);

static void chopper_quotation(chopper_t* chopper, parts_t* parts);

static void chopper_reset(chopper_t* chopper);

static bool chopper_mark(char character);

/* function that creates a "parts_t" object */
parts_t* parts_create() {

    parts_t* parts;

    parts = malloc(sizeof(*parts));

    parts->parts_size = 0;
    parts->parts_limit = MEMORY_SIZE;
    parts->parts_memory = malloc(parts->parts_limit);

    parts->size_indexes = 0;
    parts->limit_indexes = STRIDE_SIZE;
    parts->list_indexes = calloc(
        parts->limit_indexes,
        sizeof(*parts->list_indexes)
    );

    return parts;

}

/* function that releases the memory held by a "parts_t" object */
void parts_destroy(parts_t* parts) {

    free(parts->parts_memory);
    free(parts->list_indexes);
    free(parts);

}

/* function that chops a line and puts the pieces in a "parts_t" object*/
void parts_process(parts_t* parts, const char* line) {

    chopper_process(parts, line);

}

/* function that gets one part from a "parts_t" object */
char* parts_get(parts_t* parts, size_t index) {

    size_t part_index;

    if (index >= parts->size_indexes) {

        return NULL;

    }

    part_index = parts->list_indexes[index];

    return &parts->parts_memory[part_index];

}

/* function that flushes pending parts into the "parts_t" object */
static void chopper_flush(chopper_t* chopper, parts_t* parts) {

    char* pointer_source;
    char* pointer_target;
    size_t next_size;

    pointer_source = &chopper->input_line[chopper->index_source];
    pointer_target = &parts->parts_memory[chopper->index_target];

    next_size = parts->parts_size + chopper->part_length + 1;

    /* increase buffer size when there's no more space for characters */
    if (next_size > parts->parts_limit) {

        parts->parts_limit += MEMORY_SIZE;
        parts->parts_memory = realloc(
            parts->parts_memory,
            parts->parts_limit * sizeof(*parts->parts_memory)
        );

    }

    /* increase list when there's no more space for storing parts */
    if (parts->size_indexes == parts->limit_indexes) {

        parts->limit_indexes += STRIDE_SIZE;
        parts->list_indexes = realloc(
            parts->list_indexes,
            parts->limit_indexes * sizeof(*parts->list_indexes)
        );

    }

    /* ignore empty parts */
    if (chopper->part_length == 0) {

        chopper_reset(chopper);
        return;

    }

    /* copies the chunk from the source buffer to the target buffer */
    strncpy(pointer_target, pointer_source, chopper->part_length);

    /* places null character to mark the end of the string */
    pointer_target[chopper->part_length] = '\0';

    parts->list_indexes[parts->size_indexes] = chopper->index_target;
    parts->size_indexes++;

    parts->parts_size += chopper->part_length + 1;
    chopper->index_target += chopper->part_length + 1;

}

/* function that processes a line into parts */
static void chopper_process(parts_t* parts, const char* line) {

    char character;
    chopper_t chopper;

    /* reset the parts object */
    parts->size_indexes = 0;
    parts->parts_size = 0;

    chopper.quotation = '\0';
    chopper.part_length = 0;

    chopper.index_source = 0;
    chopper.index_target = 0;

    chopper.input_line = (char*) line;
    chopper.index_iterator = 0;

    while (true) {

        character = chopper.input_line[chopper.index_iterator];

        /* end of string found, break loop */
        if (character == '\0' || character == ';') {

            break;

        }

        /* checks for double quotes or single quotes */
        if (character == '"' || character == '\'') {

            if (chopper.quotation == character) { /* found closing quote */

                chopper.quotation = '\0';

                chopper_quotation(&chopper, parts);

                continue;

            }

            if (chopper.quotation == '\0') { /* found opening quote */

                chopper.quotation = character;

                chopper_quotation(&chopper, parts);

                continue;

            }

        }

        /* when a space is found outside quotes, flush and reset */
        if (isspace(character) && chopper.quotation == '\0') {

            chopper_flush(&chopper, parts);

            chopper.index_iterator++;

            chopper_reset(&chopper);

            continue;

        }

        if (chopper_mark(character)) {

            /* flushes parts before marks */
            chopper_flush(&chopper, parts);

            /* flushes mark */
            chopper.index_source = chopper.index_iterator;
            chopper.part_length = 1;
            chopper_flush(&chopper, parts);

            chopper.index_iterator++;

            chopper_reset(&chopper);

            continue;

        }

        chopper.index_iterator++;
        chopper.part_length++;

    }

    chopper_flush(&chopper, parts);

}

/* function that handles parts inside quotes */
static void chopper_quotation(chopper_t* chopper, parts_t* parts) {

    /* flushes parts before quotation marks */
    chopper_flush(chopper, parts);

    /* flushes the quotation mark */
    chopper->index_source = chopper->index_iterator;
    chopper->part_length = 1;
    chopper_flush(chopper, parts);

    chopper->index_iterator++;

    chopper_reset(chopper);

}

/* function that moves the finite-state machine to the initial state */
static void chopper_reset(chopper_t* chopper) {

    chopper->index_source = chopper->index_iterator;
    chopper->part_length = 0;

}

static const char* marks = ".,:;#()[]";

/* function that tests if a given character is a special mark */
static bool chopper_mark(char character) {

    char mark;
    char* iterator;

    iterator = (char*) marks;
    while (*iterator != '\0') {

        mark = *iterator;

        if (mark == character) {

            /* mark found */
            return true;

        }

        iterator++;

    }

    /* no mark found */
    return false;

}
