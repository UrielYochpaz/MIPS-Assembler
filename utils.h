#ifndef ASSEMBLER2_UTILS_H
#define ASSEMBLER2_UTILS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "constants.h"

extern const char *reserved[];

extern int lineNumber;

/* Used for conversion to binary base */
void swap(char *x, char *y);
char* reverse(char *buffer, int i, int j);

void sliceString(const char *str,char *result, int start,int end);

void removeSpaces(char* s);

void convertTwosComplement(int value, char* buffer, int length);

char* itoa(int value, char* buffer, int base, int length);

int isImmediate(char *immed);

void trimNewLine(char *currentLine, unsigned long size);

int isWhiteSpace(char c);

int findFirstNonWhiteSpace(const char *str);

int findFirstWhiteSpace(const char *str);

int checkLabel(char *name);

int hexify(char *bin);

int countCommas(char * line);

void clearLineNumber();

#endif
