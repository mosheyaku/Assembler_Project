#include "instruction.h"

/* array with all possible instructions */
const instruction_t instructions[] = {
    {  0, INSTRUCTION_OPS_2,  "mov" },
    {  1, INSTRUCTION_OPS_2,  "cmp" },
    {  2, INSTRUCTION_OPS_2,  "add" },
    {  3, INSTRUCTION_OPS_2,  "sub" },
    {  4, INSTRUCTION_OPS_2,  "not" },
    {  5, INSTRUCTION_OPS_1,  "clr" },
    {  6, INSTRUCTION_OPS_1,  "lea" },
    {  7, INSTRUCTION_OPS_1,  "inc" },
    {  8, INSTRUCTION_OPS_1,  "dec" },
    {  9, INSTRUCTION_OPS_J,  "jmp" },
    { 10, INSTRUCTION_OPS_J,  "bne" },
    { 11, INSTRUCTION_OPS_1,  "red" },
    { 12, INSTRUCTION_OPS_1,  "prn" },
    { 13, INSTRUCTION_OPS_J,  "jsr" },
    { 14, INSTRUCTION_OPS_0,  "rts" },
    { 15, INSTRUCTION_OPS_0, "stop" },
    {  0,                 0,   NULL }
};