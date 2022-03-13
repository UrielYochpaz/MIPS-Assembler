#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

/* struct of data block */
typedef struct DataLine {
    char * value;
    int DC; /* data counter */
    int type;
    /* 0 - character
     * 1 - byte
     * 2 - half-word
     * 4 - word */
    struct DataLine* next;
} DataLine;

/* global linked list struct of Data Block */
extern DataLine *headData;
extern DataLine *currentData;
extern int globalDC;

int checkGuideword(char * guideWord);

int getAttributeForGuide(char * guideWord);

char * getSymbolOfExternLine(char* line,char* guideword);

void addDataToDataLine(char* value, int howMuchToIncreaseDC, int type);

int handleNumbersInGuidance(char* line,char* guideword);

int handleAscizInGuidance(char* line,char* guideword);

void updateDC(int ICF);

char * getSymbolOfEntryLine(char* line,char* guideword);

void dataToObjectFile(FILE *fptr);

int getIndexOfLast(char *s,char c);

int getIndexOfFirst(char *s,char c);

void clearDataNodes();