#include <stdio.h>
#include <string.h>

#include "booleans.h"
#include "parts.h"
#include "translator.h"

#define REGISTER_FIRST                  ( 0)
#define REGISTER_LAST                   ( 7)

static const char* forbidden[] = {
     "data",   "string",    "entry", "endmacro",
   "extern",      "mov",      "cmp",      "add",
      "sub",      "lea",      "clr",      "not",
      "inc",      "dec",      "jmp",      "bne",
      "jsr",      "get",      "prn",      "rts",
      "hlt",    "macro",       NULL
};

static bool translator_forbidden(const char* word);

static instruction_t* translator_instruction(const char* word);

static bool translator_operand(
    operand_t* operand,
    parts_t* parts,
    size_t base_limit,
    size_t base_index
);

static bool translator_register(
    operand_t* operand,
    const char* token,
    int* register_value
);

static bool translator_value(
    operand_t* operand,
    const char* token,
    int* output_value
);

static void translator_action(
    translator_t* translator,
    const char* line,
    void* context,
    translator_error callback_error,
    translator_line callback_line,
    size_t base_index,
    size_t base_limit
);

/* function that creates a "translator_t" object */
translator_t* translator_create() {

    translator_t* translator;

    translator = malloc(sizeof(*translator));
    translator->parts = parts_create();

    return translator;

}

/* function that releases the memory held by a "translator_t" object */
void translator_destroy(translator_t* translator) {

    free(translator);

}

/* function that runs the process of creating lines from source file */
void translator_process(
    translator_t* translator,
    const char* line,
    void* context,
    translator_error do_error,
    translator_line do_line
) {

    translator_action(translator, line, context, do_error, do_line, 0, 0);

}

/* function that processes lines and find macros and instructions */
static void translator_action(
    translator_t* translator,
    const char* buffer,
    void* context,
    translator_error callback_error,
    translator_line callback_line,
    size_t base_index,
    size_t base_limit
) {

    int result;
    int value;

    int* size_numbers;
    int* list_numbers;

    size_t index;

    char* token;

    parts_t* parts;
    line_t line;
    instruction_t* instruction;

    parts = translator->parts;
    parts_process(parts, buffer);

    base_index = 0;
    base_limit = parts->size_indexes;

    while (base_limit >= 2) {

        token = parts_get(parts, base_index + 1);
        if (strcmp(token, ":") == 0) {

            token = parts_get(parts, base_index + 0);
            if (translator_forbidden(token)) {

                callback_error(context, 1);
                return;

            }

            token = parts_get(parts, base_index + 0);
            line.type = LINE_LABEL;
            line.data.label.name = token;
            callback_line(context, &line);

            base_index += 2;
            base_limit -= 2;
            continue;

        }

        break;

    }

    if (base_limit == 0) {

        return;

    }

    token = parts_get(parts, base_index + 0);
    instruction = translator_instruction(token);
    if (instruction != NULL) { /* instruction found */

        line.type = LINE_INSTRUCTION;
        line.data.instruction.name = instruction->name;
        line.data.instruction.opcode = instruction->opcode;
        line.data.instruction.type = instruction->type;
        line.data.instruction.operand_a.type = 0;
        line.data.instruction.operand_a.memory_label = NULL;
        line.data.instruction.operand_b.type = 0;
        line.data.instruction.operand_b.memory_label = NULL;
        line.data.instruction.param_1.type = 0;
        line.data.instruction.param_1.memory_label = NULL;
        line.data.instruction.param_2.type = 0;
        line.data.instruction.param_2.memory_label = NULL;

        switch (instruction->type) {

            case INSTRUCTION_OPS_2: {

                bool valid_operand;

                for (index = 0; index < base_limit; index++) {

                    token = parts_get(parts, base_index + index);

                    if (strcmp(token, ",") == 0) {

                        break;

                    }

                }

                valid_operand = translator_operand(
                    &line.data.instruction.operand_a,
                    parts,
                    index - 1,
                    base_index + 1
                );

                if (!valid_operand) {

                    goto failure_instruction;

                }

                valid_operand = translator_operand(
                    &line.data.instruction.operand_b,
                    parts,
                    base_limit - (index + 1),
                    base_index + (index + 1)
                );

                if (!valid_operand) {

                    goto failure_instruction;

                }

                callback_line(context, &line);
                return;

            }

            case INSTRUCTION_OPS_1: {

                bool valid_operand;

                valid_operand = translator_operand(
                    &line.data.instruction.operand_a,
                    parts,
                    base_limit - 1,
                    base_index + 1
                );

                if (!valid_operand) {

                    goto failure_instruction;

                }

                callback_line(context, &line);
                return;

            }

            case INSTRUCTION_OPS_0: {

                callback_line(context, &line);
                return;

            }

            case INSTRUCTION_OPS_J: {

                bool valid_operand;

                token = parts_get(parts, base_index + 0);

                valid_operand = translator_operand(
                    &line.data.instruction.operand_a,
                    parts,
                    1,
                    base_index + 1
                );

                if (!valid_operand) {

                    goto failure_instruction;

                }

                if (line.data.instruction.operand_a.type != OP_DIRECT_MEMORY) {

                    goto failure_instruction;

                }

                line.data.instruction.operand_a.type = OP_JUMP_PARAMETERS;

                if (base_index + 2 == base_limit) {

                    line.data.instruction.jump_short = true;
                    callback_line(context, &line);

                    return;

                }

                line.data.instruction.jump_short = false;

                token = parts_get(parts, base_index + 2);

                if (strcmp(token, "(") != 0) {

                    goto failure_instruction;

                }

                token = parts_get(parts, base_limit - 1);

                if (strcmp(token, ")") != 0) {

                    goto failure_instruction;

                }

                for (index = base_index + 2; index < base_limit; index++) {

                    token = parts_get(parts, base_index + index);

                    if (strcmp(token, ",") == 0) {

                        break;

                    }

                }

                valid_operand = translator_operand(
                    &line.data.instruction.param_1,
                    parts,
                    index - (base_index + 3),
                    base_index + 3
                );

                if (!valid_operand) {

                    goto failure_instruction;

                }

                valid_operand = translator_operand(
                    &line.data.instruction.param_2,
                    parts,
                    (base_limit - 2) - index,
                    index + 1
                );

                if (!valid_operand) {

                    goto failure_instruction;

                }

                callback_line(context, &line);
                return;

            }

        }

        failure_instruction: {

            callback_error(context, 2);
            return;

        }

    }

    token = parts_get(parts, base_index + 0);

    if (strcmp(token, ".") == 0) { /* directive */

        if (base_limit < DIRECTIVE_TOKENS_FLOOR) {

            callback_error(context, 3);
            return;

        }

        token = parts_get(parts, base_index + 1);

        if (strcmp(token, "data") == 0) {

            line.type = LINE_DIRECTIVE_DATA;
            line.data.directive_data.size_numbers = 0;
            line.data.directive_data.list_numbers = calloc(
                (base_limit - 2),
                sizeof(*line.data.directive_data.list_numbers)
            );

            size_numbers = &line.data.directive_data.size_numbers;
            list_numbers = line.data.directive_data.list_numbers;

            /* wrong number of tokens, error */
            if (base_limit % 2 == 0) {

                goto failure_directive_data;

            }

            for (index = 2; index < base_limit; index++) {

                token = parts_get(parts, base_index + index);

                if (index % 2 != 0) { /* commas on odd indexes */

                    if (strcmp(token, ",") != 0) {

                        goto failure_directive_data;
                        
                    }

                } else { /* numbers on even indexes */

                    result = sscanf(token, "%d", &value);

                    if (result != 1) {

                        goto failure_directive_data;

                    }

                    /* adds value to the list of directive data numbers */
                    *list_numbers = value;

                    (*size_numbers)++;
                    list_numbers++;

                }

            }

            callback_line(context, &line);
            free(line.data.directive_data.list_numbers);
            return;

            failure_directive_data: {

                free(line.data.directive_data.list_numbers);
                callback_error(context, 4);
                return;

            }

        }

        if (strcmp(token, "string") == 0) {

            token = parts_get(parts, base_index + 3);

            line.type = LINE_DIRECTIVE_STRING;
            line.data.directive_string.data = token;

            if (base_limit != 5) {

                goto failure_directive_string;

            }

            token = parts_get(parts, base_index + 2);
            if (strcmp(token, "\"") != 0) {

                goto failure_directive_string;

            }

            token = parts_get(parts, base_index + 4);
            if (strcmp(token, "\"") != 0) {

                goto failure_directive_string;

            }

            callback_line(context, &line);
            return;

            failure_directive_string: {

                callback_error(context, 5);
                return;

            }

        }

        if (strcmp(token, "entry") == 0) {

            token = parts_get(parts, base_index + 2);

            line.type = LINE_DIRECTIVE_ENTRY;
            line.data.directive_entry.label = token;

            if (base_limit != 3) {

                goto failure_directive_entry;

            }

            if (translator_forbidden(token)) {

                goto failure_directive_entry;

            }

            callback_line(context, &line);
            return;

            failure_directive_entry: {

                callback_error(context, 7);
                return;

            }

        }

        if (strcmp(token, "extern") == 0) {

            token = parts_get(parts, base_index + 2);

            line.type = LINE_DIRECTIVE_EXTERN;
            line.data.directive_string.data = token;

            if (base_limit != 3) {

                goto failure_directive_extern;

            }

            if (translator_forbidden(token)) {

                goto failure_directive_extern;

            }

            callback_line(context, &line);
            return;

            failure_directive_extern: {

                callback_error(context, 8);
                return;

            }

        }

    }

    switch (base_limit) {

        case 1: {

            token = parts_get(parts, base_index + 0);

            if (strcmp(token, "endmacro") == 0) { /* end of macro */

                line.type = LINE_MACRO_END;

                callback_line(context, &line);
                return;

            }

            if (!translator_forbidden(token)) { /* macro invocation */

                line.type = LINE_MACRO_INVOCATION;
                line.data.macro_invocation.name = token;

                callback_line(context, &line);
                return;

            }

            callback_error(context, 9);
            return;

        }

        case 2: {

            token = parts_get(parts, base_index + 0);

            if (strcmp(token, "macro") == 0) { /* begin of macro */

                token = parts_get(parts, base_index + 1);

                if (!translator_forbidden(token)) { /* proper name for macro */

                    line.type = LINE_MACRO_BEGIN;
                    line.data.macro_begin.name = token;

                    callback_line(context, &line);
                    return;

                }

            }

            callback_error(context, 10);
            return;

        }

    }

    callback_error(context, 11);
    return;

}

/* function that checks if a word is forbidden to be used as an identifier */
static bool translator_forbidden(const char* word) {

    char** iterator;

    iterator = (char**) forbidden;
    while (*iterator != NULL) {

        if (strcmp(*iterator, word) == 0) {

            return true;

        }

        iterator++;

    }

    return false;

}

/* function that tries to find the instruction with the given name */
static instruction_t* translator_instruction(const char* word) {

    instruction_t* iterator;

    iterator = (instruction_t*) instructions;

    while (true) {

        /* found the end of the list, break loop */
        if (iterator->name == NULL) {

            break;

        }

        if (strcmp(iterator->name, word) == 0) {

            return iterator;

        }

        iterator++;

    }

    return NULL;

}

/* function that processes the parts to find out the operand */
static bool translator_operand(
    operand_t* operand,
    parts_t* parts,
    size_t base_limit,
    size_t base_index
) {

    char* token;
    size_t index;

    operand->type = (-1);
    operand->cpu_register = 0;
    operand->value_immediate = 0;
    operand->memory_label = NULL;
    operand->structure = false;
    operand->structure_index = (-1);

    for (index = 0; index < base_limit; index++) {

        token = parts_get(parts, base_index + index);

    }

    switch (base_limit) {

        case 1: {

            token = parts_get(parts, base_index + 0);

            if (translator_forbidden(token)) {

                return false;

            }

            if (translator_register(operand, token, &operand->cpu_register)) {

                operand->type = OP_DIRECT_REGISTER;
                return true;

            }

            operand->type = OP_DIRECT_MEMORY;
            operand->memory_label = token;
            return true;

        }

        case 2: {

            token = parts_get(parts, base_index + 0);

            if (strcmp(token, "#") != 0) {

                return false;

            }

            token = parts_get(parts, base_index + 1);

            if (translator_value(operand, token, &operand->value_immediate)) {

                operand->type = OP_DIRECT_IMMEDIATE;
                return true;

            }

            return false;

        }

    }

    return false;

}

/* function that processes the token to find the register */
static bool translator_register(
    operand_t* operand,
    const char* token,
    int* value_register
) {

    int result, value;

    /* a register declaration must start with a lowercase 'r' */
    if (token[0] != 'r') {

        return false;

    }

    result = sscanf(token + 1, "%d", &value);

    /* failed to read number of register */
    if (result != 1) {

        return false;

    }

    /* the register number is outside the boundaries, fail */
    if (value < REGISTER_FIRST || REGISTER_LAST < value) {

        return false;

    }

    *value_register = value;
    return true;

}

/* function to find the immediate value of a token */
static bool translator_value(
    operand_t* operand,
    const char* token,
    int* output_value
) {

    int result, value;

    result = sscanf(token, "%d", &value);

    /* failed to read immediate value */
    if (result != 1) {

        return false;

    }

    *output_value = value;
    return true;

}
