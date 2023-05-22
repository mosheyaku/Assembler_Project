#include <stdlib.h>

#include "line.h"
#include "helper.h"

/* function that clones a "line_t" object */
line_t* line_clone(line_t* line) {

    line_t* clone;

    clone = malloc(sizeof(*clone));
    clone->type = line->type;

    switch (line->type) {

        case LINE_LABEL: {

            clone->data.label.name = helper_clone_string(
                line->data.label.name
            );

            break;

        }

        case LINE_DIRECTIVE_DATA: {

            int size_numbers;
            int single_number;
            size_t index;

            size_numbers = line->data.directive_data.size_numbers;
            clone->data.directive_data.size_numbers = size_numbers;
            clone->data.directive_data.list_numbers = calloc(
                clone->data.directive_data.size_numbers,
                sizeof(*clone->data.directive_data.list_numbers)
            );

            for (index = 0; index < size_numbers; index++) {

                single_number = line->data.directive_data.list_numbers[index];
                clone->data.directive_data.list_numbers[index] = single_number;

            }

            break;

        }

        case LINE_DIRECTIVE_ENTRY: {

            clone->data.directive_entry.label = helper_clone_string(
                line->data.directive_entry.label
            );

            break;

        }

        case LINE_DIRECTIVE_EXTERN: {

            clone->data.directive_extern.label = helper_clone_string(
                line->data.directive_extern.label
            );

            break;

        }

        case LINE_DIRECTIVE_STRING: {

            clone->data.directive_string.data = helper_clone_string(
                line->data.directive_string.data
            );

            break;

        }

        case LINE_INSTRUCTION: {

            clone->data.instruction = line->data.instruction;

            clone->data.instruction.operand_a.memory_label = helper_clone_string(
                line->data.instruction.operand_a.memory_label
            );

            clone->data.instruction.operand_b.memory_label = helper_clone_string(
                line->data.instruction.operand_b.memory_label
            );

            clone->data.instruction.param_1.memory_label = helper_clone_string(
                line->data.instruction.param_1.memory_label
            );

            clone->data.instruction.param_2.memory_label = helper_clone_string(
                line->data.instruction.param_2.memory_label
            );

            break;

        }

    }

    return clone;

}

/* function that releases the memory held by a "line_t" object */
void line_destroy(line_t* line) {

    switch (line->type) {

        case LINE_LABEL: {

            free(line->data.label.name);

            break;

        }

        case LINE_DIRECTIVE_DATA: {

            free(line->data.directive_data.list_numbers);

            break;

        }

        case LINE_DIRECTIVE_ENTRY: {

            free(line->data.directive_entry.label);

            break;

        }

        case LINE_DIRECTIVE_EXTERN: {

            free(line->data.directive_extern.label);

            break;

        }

        case LINE_DIRECTIVE_STRING: {

            free(line->data.directive_string.data);

            break;

        }

        case LINE_INSTRUCTION: {

            free(line->data.instruction.operand_a.memory_label);
            free(line->data.instruction.operand_b.memory_label);
            free(line->data.instruction.param_1.memory_label);
            free(line->data.instruction.param_2.memory_label);

            break;

        }

    }

    free(line);

}