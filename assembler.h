#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "guideUtils.h"
#include "instructionUtils.h"
#include "utils.h"
#include "constants.h"

/* struct of symbol node (symbol table) */
typedef struct SymbolNode {
    char * name; /* label name */
    int value; /* (decimal) according to symbol type */
    /* attribute options -
     * code :      0
     * data :      1
     * external:   2
     * code,entry: 3
     * data,entry: 4
     */
    int attribute;
    struct SymbolNode * next;
} SymbolNode;

typedef struct EntryNode {
    char * data; /* label name */
    int line;
    struct EntryNode * next;
} EntryNode;
extern EntryNode *headEntry;
extern EntryNode *currentEntry;

/* Global linked lists pointers */
extern SymbolNode *headSymbol;
extern SymbolNode *currentSymbol;
extern int ICF;
extern int DCF;

int main(int argc, char* argv[]);

int assemble(FILE* fp, char* fileName);

int parseLine(char *line);

int parseLine2(char *line,char * theLabel);

int addSymbol(char *name, int value, int attribute);

int addLabelToEntryNode(char*  theLabel);

void updateSymbols();

int labelFiller(char *name);

void writeEntryFile(char *file);

int checkForEntries();

void writeOBFile(char *file);

char *get_filename_ext(const char *filename);

void clearSymbolTable();

void clearEntryNodes();