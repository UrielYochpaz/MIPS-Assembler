#pragma once

typedef struct {
    char *name;
    const char type[2];
    int opcode;
    int funct;
} Command;

static Command validInstructions[] = {
        {"add", "R", 0, 1},
        {"sub", "R", 0, 2},
        {"and", "R", 0, 3},
        {"or", "R", 0, 4},
        {"nor", "R", 0, 5},
        {"move", "R", 1, 1},
        {"mvhi", "R", 1, 2},
        {"mvlo", "R", 1, 3},
        {"addi", "I", 10, 0},
        {"subi", "I", 11, 0},
        {"andi", "I", 12, 0},
        {"ori", "I", 13, 0},
        {"nori", "I", 14 , 0},
        {"bne", "I", 15, 0},
        { "beq", "I", 16, 0},
        { "blt", "I", 17, 0},
        { "bgt", "I", 18, 0},
        { "lb", "I", 19, 0},
        { "sb", "I", 20, 0},
        { "lw", "I", 21, 0},
        { "sw", "I", 22, 0},
        { "lh", "I", 23, 0},
        { "sh", "I", 24, 0},
        { "jmp", "J", 30, 0 },
        { "la", "J", 31, 0 },
        { "call", "J", 32, 0 },
        { "stop", "J", 63, 0 },
};