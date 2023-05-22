#pragma once

#include <stdlib.h>

#define INSTRUCTION_OPS_2        (1)
#define INSTRUCTION_OPS_1        (2)
#define INSTRUCTION_OPS_0        (3)
#define INSTRUCTION_OPS_J        (4)

/* structure that represents a single instruction */
typedef struct instruction {
    int opcode;
    int type;
    char* name;
} instruction_t;

extern const instruction_t instructions[];
