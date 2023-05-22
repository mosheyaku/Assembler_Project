#pragma once

#include "booleans.h"
#include "instruction.h"

#define LINE_NONE                   ( 0)
#define LINE_LABEL                  ( 1)
#define LINE_MACRO_BEGIN            ( 2)
#define LINE_MACRO_END              ( 3)
#define LINE_MACRO_INVOCATION       ( 4)
#define LINE_DIRECTIVE_DATA         ( 5)
#define LINE_DIRECTIVE_STRING       ( 6)
#define LINE_DIRECTIVE_ENTRY        ( 7)
#define LINE_DIRECTIVE_EXTERN       ( 8)
#define LINE_INSTRUCTION            ( 9)

#define OP_DIRECT_IMMEDIATE        (0)
#define OP_DIRECT_MEMORY           (1)
#define OP_JUMP_PARAMETERS         (2)
#define OP_DIRECT_REGISTER         (3)

#define DIRECTIVE_TOKENS_FLOOR          (3)

/* structure that represents one operand of an instruction */
typedef struct operand {
    int type; /* 0, 1, 2, 3 */
    bool structure;

    int value_immediate;
    int cpu_register;
    char* memory_label;
    int structure_index;
} operand_t;

/* union of all possible logical shapes that a line of code can take */
typedef union line_union {

    struct {
        char* name;
    } label;

    struct {
        char* name;
    } macro_begin;

    struct {
        char* name;
    } macro_invocation;

    struct {
        int size_numbers;
        int* list_numbers;
    } directive_data;

    struct {
        char* data;
    } directive_string;

    struct {
        char* label;
    } directive_entry;

    struct {
        char* label;
    } directive_extern;

    struct {
        const char* name;
        int opcode;
        int type; /* 0, 1, 2, 3 */
        operand_t operand_a;
        operand_t operand_b;
        operand_t param_1;
        operand_t param_2;
        bool jump_short;
    } instruction;

} line_union_t;

/* structure that represents a single line */
typedef struct line {
    int type;
    line_union_t data;
} line_t;

line_t* line_clone(line_t* line);

void line_destroy(line_t* line);