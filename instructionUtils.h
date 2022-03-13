#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "opcodes.h"
#include "utils.h"
#include "constants.h"

/* struct of instruction block */
typedef struct InstructionLine {
    char value[COMMAND_SIZE]; /* binary encoding of instruction */
    int IC; /* instruction counter */
    int line; /* The instruction's line number*/
    char type[2]; /* Instruction type */
    char *label;
    struct InstructionLine* next;
} InstructionLine;

extern InstructionLine *headInstruction;
extern InstructionLine *currentInstruction;
extern int globalIC;

int handleInstruction(char *instructionName, char* line);

Command* checkInstruction(char *instructionName, char* line);

int checkOperators(char* instructionName, char* line, Command *metaCommand);

int isRegister(char *token, int errorPrint);

void addInstructionToInstructionLine(Command *metaCommand, size_t m, char registers[][m], char *immede, char *label);

void fillValue(Command *metaCommand, size_t m, char registers[][m], char *immede, InstructionLine *node);

void instructionsToObjectFile(FILE *fptr, int icf, int dcf);

void clearInstructionNodes();