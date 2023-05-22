#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "booleans.h"
#include "builder.h"
#include "helper.h"
#include "translator.h"

#define LINE_LIMIT               (256)

#define CHUNK_LIMIT (16)

#define ATTR_ABSOLUTE                 (0)
#define ATTR_EXTERNAL                 (1)
#define ATTR_RELOCATABLE              (2)

#define PRINT_ENT "\t%s\t%d\n"

#define PRINT_EXT "\t%s\t%d\n"

#define HEADER_OB "\t%d\t%d\n"
#define PRINT_OB "\t%04d\t%s\n"

#define MASK_BASE (0xFFF0)
#define MASK_OFFSET (0x000F)

static char helper_digits[LIMIT_BINARY + 1];

/* function that writes one line to the ".ent" file */
static void builder_output_ent(
    builder_t* builder,
    const char* string,
    int program_counter
) {

    fprintf(
        builder->file_ent,
        PRINT_ENT,
        string,
        program_counter
    );
    
}

/* function that writes one line to the ".ext" file */
static void builder_output_ext(
    builder_t* builder,
    const char* string,
    int program_counter
) {

    fprintf(
        builder->file_ext,
        PRINT_EXT,
        string,
        program_counter
    );

}

/* function that writes one line to the ".ob" file */
static void builder_output_ob(
    builder_t* builder,
    int value
) {

    value = value & 0x3FFF;

    helper_encode_binary(value, helper_digits);

    fprintf(
        builder->file_ob,
        PRINT_OB,
        (int) builder->program_counter,
        helper_digits
    );

    builder->program_counter++;

}

/* function that writes one line to the ".ob" file, given the register pair */
static void builder_output_register_pair(
    builder_t* builder,
    operand_t* operand_a,
    operand_t* operand_b
) {

    int word;

    word = 0;
    word = word | operand_a->cpu_register << 8;
    word = word | operand_b->cpu_register << 2;

    builder_output_ob(
        builder,
        word | ATTR_ABSOLUTE
    );

}

/* function that writes one line to the ".ob" file, given the operand */
static void builder_output_operand(
    builder_t* builder,
    operand_t* operand,
    int order /* 0 == 'operand_a', 1 == 'operand_b' */
) {

    char* string;
    size_t index;
    label_t* label;

     switch (operand->type) {

        case OP_DIRECT_IMMEDIATE: {

            builder_output_ob(
                builder,
                ((int) operand->value_immediate << 2) | ATTR_ABSOLUTE
            );

            break;

        }

        case OP_DIRECT_MEMORY:
        case OP_JUMP_PARAMETERS: {

            string = operand->memory_label;
            label = NULL;

            for (index = 0; index < builder->size_labels; index++) {

                label = builder->list_labels[index];

                if (strcmp(label->name, string) == 0) {

                    break;

                }

                label = NULL;

            }

            if (label != NULL) {

                builder_output_ob(
                    builder,
                    label->full_address << 2 | ATTR_RELOCATABLE
                );

            }

            if (label == NULL) {

                builder_output_ext(
                    builder,
                    string,
                    builder->program_counter
                );

                builder_output_ob(
                    builder,
                    0 | ATTR_EXTERNAL
                );

            }

            break;

        }

        case OP_DIRECT_REGISTER: {

            int shift;

            shift = order == 0 ? 8 : 2;
            builder_output_ob(
                builder,
                (operand->cpu_register << shift) | ATTR_ABSOLUTE
            );
            
            break;

        }

    }

}

/* function that writes to the ".ent", ".ext" and ".ob" files, given a line */
static void builder_output(builder_t* builder, line_t* line) {

    int value;
    int type;
    char* string;
    size_t index;
    label_t* label;
    operand_t* operand_a;
    operand_t* operand_b;
    operand_t* param_1;
    operand_t* param_2;
    unsigned char letter;

    switch (line->type) {

        case LINE_DIRECTIVE_DATA: {

            int size_numbers;
            int* list_numbers;

            size_numbers = line->data.directive_data.size_numbers;
            list_numbers = line->data.directive_data.list_numbers;

            for (index = 0; index < size_numbers; index++) {

                value = list_numbers[index];

                builder_output_ob(
                    builder,
                    value | ATTR_ABSOLUTE
                );

            }

            break;

        }

        case LINE_DIRECTIVE_ENTRY: {

            string = line->data.directive_string.data;
            label = NULL;

            for (index = 0; builder->size_labels; index++) {

                label = builder->list_labels[index];

                if (strcmp(label->name, string) == 0) {

                    break;

                }

                label = NULL;

            }

            if (label != NULL) {

                label->entry = true;

            }

            break;

        }

        case LINE_DIRECTIVE_EXTERN: {

            break;

        }

        case LINE_DIRECTIVE_STRING: {

            string = line->data.directive_string.data;

            while (true) {

                letter = (unsigned char) *string;

                builder_output_ob(
                    builder,
                    letter | ATTR_ABSOLUTE
                );

                if (letter == '\0') {

                    break;

                }

                string++;

            }

            break;

        }

        case LINE_INSTRUCTION: {

            int opcode;
            int word;

            type = line->data.instruction.type;
            opcode = line->data.instruction.opcode;
            operand_a = &line->data.instruction.operand_a;
            operand_b = &line->data.instruction.operand_b;
            param_1 = &line->data.instruction.param_1;
            param_2 = &line->data.instruction.param_2;

            word = 0;
            word = word | line->data.instruction.param_1.type << 12;
            word = word | line->data.instruction.param_2.type << 10;
            word = word | line->data.instruction.opcode << 6;

            if (opcode == 0) {

                word = word | line->data.instruction.operand_a.type << 4;
                word = word | line->data.instruction.operand_b.type << 2;

            } else {

                word = word | line->data.instruction.operand_b.type << 4;
                word = word | line->data.instruction.operand_a.type << 2;

            }

            builder_output_ob(
                builder,
                word | ATTR_ABSOLUTE
            );

            switch (type) {

                case INSTRUCTION_OPS_2: {

                    int test_a;
                    int test_b;

                    test_a = operand_a->type == OP_DIRECT_REGISTER;
                    test_b = operand_b->type == OP_DIRECT_REGISTER;

                    if (test_a && test_b) {

                        builder_output_register_pair(
                            builder,
                            operand_a,
                            operand_b
                        );

                        break;

                    }

                    builder_output_operand(
                        builder,
                        operand_a,
                        0
                    );

                    builder_output_operand(
                        builder,
                        operand_b,
                        1
                    );

                    break;

                }

                case INSTRUCTION_OPS_1: {

                    builder_output_operand(
                        builder,
                        operand_a,
                        0
                    );

                    break;

                }

                case INSTRUCTION_OPS_0: {

                    /*  nothing to do */
                    break;

                }

                case INSTRUCTION_OPS_J: {

                    int test_a;
                    int test_b;

                    builder_output_operand(
                        builder,
                        operand_a,
                        0
                    );

                    if (line->data.instruction.jump_short) {

                        break;

                    }

                    test_a = param_1->type == OP_DIRECT_REGISTER;
                    test_b = param_2->type == OP_DIRECT_REGISTER;

                    if (test_a && test_b) {

                        builder_output_register_pair(
                            builder,
                            param_1,
                            param_2
                        );

                        break;

                    }

                    builder_output_operand(
                        builder,
                        param_1,
                        0
                    );

                    builder_output_operand(
                        builder,
                        param_2,
                        1
                    );

                    break;

                }

            }

        }

    }
    
}

/* function that calculates the size of the operand part of an instruction */
static size_t builder_size_operand(operand_t* operand) {

    switch (operand->type) {

        case OP_DIRECT_IMMEDIATE: {

            return 1;

        }

        case OP_DIRECT_MEMORY: {

            return 1;

        }

        case OP_DIRECT_REGISTER: {

            return 1;

        }

    }

    return (size_t) (-1);

}

/* function that calculates the full size in words of a given line */
static void builder_size_line(
    builder_t* builder,
    line_t* line
) {

    switch (line->type) {

        case LINE_INSTRUCTION: {

            size_t size_instruction;
            operand_t* operand_a;
            operand_t* operand_b;
            operand_t* param_1;
            operand_t* param_2;

            size_instruction = 1;

            switch (line->data.instruction.type) {

                case INSTRUCTION_OPS_2: {

                    int test_a;
                    int test_b;

                    operand_a = &line->data.instruction.operand_a;
                    operand_b = &line->data.instruction.operand_b;

                    test_a = operand_a->type == OP_DIRECT_REGISTER;
                    test_b = operand_b->type == OP_DIRECT_REGISTER;

                    if (test_a && test_b) {

                        size_instruction += 1;

                        break;

                    }

                    size_instruction += builder_size_operand(operand_a);
                    size_instruction += builder_size_operand(operand_b);

                    break;

                }

                case INSTRUCTION_OPS_1: {

                    operand_a = &line->data.instruction.operand_a;
                    size_instruction += builder_size_operand(operand_a);

                    break;

                }

                case INSTRUCTION_OPS_0: {

                    break;

                }

                case INSTRUCTION_OPS_J: {

                    int test_a;
                    int test_b;

                    size_instruction += 1;

                    if (line->data.instruction.jump_short) {

                        break;

                    }

                    param_1 = &line->data.instruction.param_1;
                    param_2 = &line->data.instruction.param_2;

                    test_a = param_1->type == OP_DIRECT_REGISTER;
                    test_b = param_2->type == OP_DIRECT_REGISTER;

                    if (test_a && test_b) {

                        size_instruction += 1;

                        break;

                    }

                    size_instruction += 2;

                    break;

                }                

            }

            builder->program_counter += size_instruction;
            builder->length_code += size_instruction;

            break;

        }

        case LINE_DIRECTIVE_DATA: {

            size_t size_data;

            size_data = line->data.directive_data.size_numbers;
            builder->program_counter += size_data;
            builder->length_data += size_data;

            break;

        }

        case LINE_DIRECTIVE_STRING: {

            size_t size_string;

            size_string = strlen(line->data.directive_string.data) + 1;
            builder->program_counter += size_string;
            builder->length_data += size_string;

            break;

        }

    }

}

/* function that outputs an error when one is found */
static void builder_error(builder_t* builder, int code) {

    builder->errors_found++;

    fprintf(
        stderr,
        "[code: %d] found error #%d on line %d\n%s\n\n",
        code,
        (int) builder->errors_found,
        (int) builder->line_number,
        builder->current_line
    );

}

/* function that processes one line from the input */
static void builder_line(builder_t* builder, line_t* line) {

    size_t index;
    char* string;
    macro_t* macro;
    label_t* label;
    line_t* clone_line;

    if (line->type == LINE_MACRO_BEGIN) {

        macro = macro_create(line->data.macro_begin.name);

        if (builder->size_macros == builder->limit_macros) {

            builder->limit_macros += CHUNK_LIMIT;
            builder->list_macros = realloc(
                builder->list_macros,
                builder->limit_macros * sizeof(*builder->list_macros)
            );

        }

        builder->list_macros[builder->size_macros] = macro;
        builder->size_macros++;

        builder->current_macro = macro;
        return;

    }

    if (line->type == LINE_MACRO_END) {

        builder->current_macro = NULL;
        return;

    }

    if (line->type == LINE_MACRO_INVOCATION) {

        string = line->data.macro_invocation.name;
        macro = NULL;

        for (index = 0; index < builder->size_macros; index++) {

            macro = builder->list_macros[index];

            if (strcmp(macro->name, string) == 0) {

                break;

            }

            macro = NULL;

        }

        if (macro == NULL) {

            builder_error(builder, 101);
            return;

        }

        for (index = 0; index < macro->size_lines; index++) {

            line = macro->list_lines[index];
            builder_line(builder, line);

        }

        return;

    }

    if (line->type == LINE_LABEL) {

        label = label_create(
            line->data.label.name,
            builder->program_counter
        );

        if (builder->size_labels == builder->limit_labels) {

            builder->limit_labels += CHUNK_LIMIT;
            builder->list_labels = realloc(
                builder->list_labels,
                builder->limit_labels * sizeof(*builder->list_labels)
            );

        }

        builder->list_labels[builder->size_labels] = label;
        builder->size_labels++;

        return;

    }

    if (builder->current_macro == NULL) { /* outside a macro */

        /* outputs current line to the ".am" file if outside macro */
        fputs(builder->current_line, builder->file_am);

        builder_size_line(builder, line);

        switch (line->type) {

            case LINE_MACRO_INVOCATION: {

                line_t* macro_line;

                macro = NULL;
                string = line->data.macro_invocation.name;

                for (index = 0; index < builder->size_macros; index++) {

                    macro = builder->list_macros[index];

                    /* found invoked macro, break loop */
                    if (strcmp(macro->name, string) == 0) {

                        break;

                    }

                    macro = NULL;

                }

                /* macro invocation without previously declared macro, error */
                if (macro == NULL) {

                    builder_error(builder, 102);
                    return;

                }

                /* execute all lines that belong to macro */
                for (index = 0; index < macro->size_lines; index++) {

                    macro_line = macro->list_lines[index];
                    builder_line(builder, macro_line);

                }

                break;

            }

            case LINE_INSTRUCTION:
            case LINE_DIRECTIVE_DATA:
            case LINE_DIRECTIVE_ENTRY:
            case LINE_DIRECTIVE_EXTERN:
            case LINE_DIRECTIVE_STRING: {

                clone_line = line_clone(line);

                if (builder->size_lines == builder->limit_lines) {

                    builder->limit_lines += CHUNK_LIMIT;
                    builder->list_lines = realloc(
                        builder->list_lines,
                        builder->limit_lines * sizeof(*builder->list_lines)
                    );

                }

                builder->list_lines[builder->size_lines] = clone_line;
                builder->size_lines++;

                if (line->type == LINE_DIRECTIVE_EXTERN) {

                    builder->length_extern++;

                }

                if (line->type == LINE_DIRECTIVE_ENTRY) {

                    builder->length_entry++;

                }

                break;

            }

        }

        return;

    } else { /* inside a macro */

        macro = builder->current_macro;
        clone_line = line_clone(line);

        if (macro->size_lines == macro->limit_lines) {

            macro->limit_lines += CHUNK_LIMIT;
            macro->list_lines = realloc(
                macro->list_lines,
                macro->limit_lines * sizeof(*macro->list_lines)
            );

        }

        macro->list_lines[macro->size_lines] = clone_line;
        macro->size_lines++;

    }

}

/* function that creates a "builder_t" object */
builder_t* builder_create(
    FILE* file_as,
    FILE* file_am,
    FILE* file_ob
) {

    builder_t* builder;

    builder = malloc(sizeof(*builder));

    builder->file_am = file_am;
    builder->file_as = file_as;
    builder->file_ob = file_ob;
    builder->file_ent = NULL;
    builder->file_ext = NULL;

    builder->size_lines = 0;
    builder->limit_lines = CHUNK_LIMIT;
    builder->list_lines = calloc(
        builder->limit_lines,
        sizeof(*builder->list_lines)
    );

    builder->size_labels = 0;
    builder->limit_labels = CHUNK_LIMIT;
    builder->list_labels = calloc(
        builder->limit_labels,
        sizeof(*builder->list_labels)
    );

    builder->size_macros = 0;
    builder->limit_macros = CHUNK_LIMIT;
    builder->list_macros = calloc(
        builder->limit_macros,
        sizeof(*builder->list_macros)
    );

    builder->current_macro = NULL;

    builder->errors_found = 0;

    builder->length_code = 0;
    builder->length_data = 0;

    builder->length_entry = 0;
    builder->length_extern = 0;

    builder->program_counter = 100;

    return builder;

}

/* function that releases the memory held by a "builder_t" object */
void builder_destroy(builder_t* builder) {

    size_t index;

    for (index = 0; index < builder->size_labels; index++) {

        label_destroy(builder->list_labels[index]);

    }

    for (index = 0; index < builder->size_macros; index++) {

        macro_destroy(builder->list_macros[index]);

    }

    for (index = 0; index < builder->size_lines; index++) {

        line_destroy(builder->list_lines[index]);

    }

    free(builder->list_lines);
    free(builder->list_labels);
    free(builder->list_macros);
    free(builder);

}

/* function that runs the first pass on the ".as" input file */
void builder_first_pass(builder_t* builder) {

    char line[LINE_LIMIT];
    translator_t* translator;

    translator = translator_create();

    builder->errors_found = 0;
    builder->line_number = 0;
    builder->current_line = line;
    builder->current_macro = NULL;

    while (!feof(builder->file_as)) {

        builder->line_number++;

        /* line = empty string */
        line[0] = '\0';

        /* retrieves one line from the ".as" file */
        fgets(line, LINE_LIMIT, builder->file_as);

        translator_process(
            translator,
            line,
            builder,
            (translator_error) builder_error,
            (translator_line) builder_line
        );

    }

    translator_destroy(translator);

}

/* functions that runs the second pass and outputs the ".ent", ".ext" and ".ob" files */
void builder_second_pass(
    builder_t* builder,
    FILE* file_ent,
    FILE* file_ext
) {

    size_t index;
    label_t* label;
    line_t* line;

    builder->file_ent = file_ent;
    builder->file_ext = file_ext;

    builder->program_counter = 100;

    /* outputs the header for the ".ob" file */
    fprintf(
        builder->file_ob,
        HEADER_OB,
        (int) builder->length_code,
        (int) builder->length_data
    );

    for (index = 0; index < builder->size_lines; index++) {

        line = builder->list_lines[index];
        builder_output(builder, line);

    }

    for (index = 0; index < builder->size_labels; index++) {

        label = builder->list_labels[index];

        if (label->entry && file_ent != NULL) {

            builder_output_ent(builder, label->name, label->full_address);

        }

    }

}
