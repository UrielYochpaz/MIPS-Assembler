/*
 * This file contains genral-use functions
 */

#include "utils.h"

/* global line number variable */
int lineNumber = 0;
const char *reserved[] = {"add", "sub", "and", "or", "nor", "move",
                    "mvhi", "mvlo", "addi", "subi", "andi", "ori",
                    "nori", "bne", "beq", "blt", "bgt", "lb", "sb",
                    "lw", "sw", "lh", "sh", "jmp", "la", "call", "stop",
                    "db", "dh", "dw", "asciz", "entry", "extern"};

/*
 * slices a string from start index to last index and store the result in result variable
 */
void sliceString(const char *str,char *result, int start, int end)
{
    int j = 0;
    for (int i = start; i < end; ++i)
        result[j++] = str[i];
    result[j] = 0;
}

/*
 * remove spaces from string
 */
void removeSpaces(char* s)
{
    const char* d = s;
    do {
        while (*d == ' ') {
            ++d;
        }
    } while (*s++ = *d++);
}

/*
 * converts an integer to it's binary encoding of it's Two's complement value in a specified length 
 */
void convertTwosComplement(int value, char* buffer, int length)
{
    if (value < 0)
    {
        itoa(abs(value) - 1, buffer, 2, length);
        for (int i = 0 ; i < length ; i++)
        {
            if (buffer[i] == '1')
                buffer[i] = ZERO_CHAR;
            else
                buffer[i] = '1';
        }
    }
    else
        itoa(value, buffer, 2, length);
}

/*
 * converts an integer to a diffenent base in a specified length
 * used to convert to an unsigned binary encoding of integer's value
 */
char* itoa(int value, char* buffer, int base, int length)
{
    /* invalid base */
    if (base < 2 || base > 32) {
        return buffer;
    }

    /* consider the absolute value of the number */
    int n = abs(value);

    int i;
    for (i = 0; i < length ; i++)
    {
        int r = n % base;
        buffer[i] = 48 + r;
        n = n / base;
    }

    /* 
     * If the base is 10 and the value is negative, the resulting string
     * is preceded with a minus sign (-)
     * With any other base, value is always considered unsigned 
     */
    if (value < 0 && base == 10) {
        buffer[i++] = '-';
    }

    /* null terminate string */
    buffer[i] = '\0';

    return buffer;
}

/*
 * Checks if an immediate string is truly immediate (a number)
 * Returns 1 if immediate, otherwise returns 0
 */
int isImmediate(char *immed)
{

    int length = strlen (immed);
    if (immed[0] == MINUS || immed[0] == PLUS || isdigit(immed[0]))
        {
        for (int i=1;i<length; i++)
        {
            if (!isdigit(immed[i]))
            {
                printf("line %d: the operand <%s> is not an immediate\n", lineNumber, immed);
                return 0; /* not immediate */
            }
        }
        return 1; /* immediate */
        }
    else
    {
        printf("line %d: the operand <%s> is not an immediate\n", lineNumber, immed);
        return 0; /* not immediate */
    }
}

/*
 * Removes \n from the line
 */
void trimNewLine(char *currentLine, unsigned long size)
{
    if (currentLine[size - 1] == '\n')
    {
        if (currentLine[size - 2] == CR)
            currentLine[size - 2] = '\0';

        currentLine[size - 1] = '\0';
    }
    else
        currentLine[size] = '\0';
}

/*
 * Checks if a character is a whitespace
 */
int isWhiteSpace(char c)
{
    return c == ' ' || c == '\t';
}

/*
 * returns the index of the first white space in the line
 */
int findFirstWhiteSpace(const char *str)
{
    int i = 0;

    while (!isWhiteSpace(str[i]))
        i++;

    return i;
}

/*
 * returns the index of the first non-white space in the line
 */
int findFirstNonWhiteSpace(const char *str)
{
    int i = 0;

    while (isWhiteSpace(str[i]))
        i++;

    return i;
}

/*
 * checked if the label is legit
 */
int checkLabel(char *name)
{
    for (int i = 0 ; i < NUM_OF_RESERVED ; i++)
    {
        if ((strcmp(name, reserved[i])) == 0)
        {
            printf("line %d: label <%s> is a reserved word\n",lineNumber, name);
            return(1);
        }
    }

    if (strlen(name) > MAX_LABEL_LENGTH)
    {
        printf("line %d: label too long <%s>\n",lineNumber, name);
        return(1);
    }
    else if (isalpha(*name) == 0)
    {
        printf("line %d: label starts with non-Alphabet character <%s>\n",lineNumber, name);
        return(1);
    }
    else
    {
        for(int i = 1; i < strlen(name); i++)
        {
            if (isalpha((int)name[i]) == 0 && isdigit((int)name[i]) == 0)
            {
                printf("line %d: invalid symbol <%s>\n",lineNumber, name);
                return(1);
            }
        }
        return(0);
    }
}

/*
 * convert a binary string to a hex value
 */
int hexify(char *bin)
{
    int num = 0;
    char *end = bin+LENGTH_OF_BYTE;
    do {
        int b;
        if(*bin=='1')
            b = 1;
        else
            b = 0;
        num = (num<<1)|b;
        bin++;
    } while (bin != end);
    return num;
}

/*
 * swapt between characters
 */
void swap(char *x, char *y) {
    char t = *x; *x = *y; *y = t;
}

/*
 * the function reverses a string
 */
char* reverse(char *buffer, int i, int j)
{
    while (i < j) {
        swap(&buffer[i++], &buffer[j--]);
    }

    return buffer;
}

/*
 * This function counts how many commans are in a string
 */
int countCommas(char * line)
{
    int c=0;
    for (int i = 0; i < strlen(line) ; ++i)
    {
        if (line[i] == COMMA)
            c++;
    }
    return c;
}

/*
 * initialize the global lineNumber to 0 for the next input file
 */
void clearLineNumber(){
    lineNumber = 0;
}
