/*
 * This file contains functions that handles directive lines
 */

#include "guideUtils.h"

/* global linked list struct of Data Block */
DataLine *headData = NULL;
DataLine *currentData;
int globalDC = 0;
int lineNumber;

/*
 * Checks if the guide word is ok
 */
int checkGuideword(char * guideWord)
{
    return ((strcmp(guideWord,ENTRY)) == 0 || (strcmp(guideWord,DB)==0)||((strcmp(guideWord,DW))==0)||((strcmp(guideWord,DH))==0)||((strcmp(guideWord,ASCIZ))==0 || ((strcmp(guideWord,EXTERN))==0 )));
}

/*
 * Returns number represents attribute (code,data,entry)
 */
int getAttributeForGuide(char * guideWord)
{

    if (strcmp(guideWord,EXTERN) == 0)
        return 2;
    if (strcmp(guideWord,DB) == 0)
        return 1;
    if (strcmp(guideWord,DW) == 0)
        return 1;
    if (strcmp(guideWord,DH) == 0)
        return 1;
    if (strcmp(guideWord,ASCIZ) == 0)
        return 1;

    return 0;
}

/*
 * Parse the text in extern line
 */
char * getSymbolOfExternLine(char* line,char* guideword)
{
    char *tempLine = (char*) malloc(strlen(line) * sizeof(char));
    strcpy(tempLine, line);
    removeSpaces(tempLine);
    char * onlyTheSymbol = (char*) malloc(strlen(tempLine) * sizeof(char));
    sliceString(tempLine,onlyTheSymbol,EXTERN_LENGTH, strlen(tempLine));
    return onlyTheSymbol;
}

/*
 * Adds the data to Dataline list and update DC value
 */
void addDataToDataLine(char* value, int howMuchToIncreaseDC, int type)
{
    if (headData == NULL)
    {
        headData = (struct DataLine*)malloc(sizeof(struct DataLine));
        headData->value = value;
        headData->DC = 0;
        globalDC += howMuchToIncreaseDC;
        headData->type = type;
        currentData = headData;
    }
    else
    {
        struct DataLine* newNode = (struct DataLine*)malloc(sizeof(struct DataLine));
        newNode->value = value;
        newNode->DC = globalDC;
        globalDC += howMuchToIncreaseDC;
        newNode->type = type;
        newNode->next = NULL;
        currentData->next = newNode;
        currentData = newNode;
    }
}

/*
 * Parse the numbers in guide line of dw,dh,db and send the data to addDataToDataLine function
 */
int handleNumbersInGuidance(char* line, char* guideword)
{
    /* here the  guideword is .db / .dw / .dh */
    /* iterate over the numbers and save in data line */

    /* removing spaces from line*/
    char *tempLine = (char*) malloc(strlen(line) * sizeof(char));
    strcpy(tempLine, line);
    removeSpaces(tempLine);
    char * onlyTheNumbers = (char*) malloc(strlen(tempLine) * sizeof(char));
    sliceString(tempLine,onlyTheNumbers,NUMBERS_LENGTH, strlen(tempLine));

    /* split the numbers by comma */
    char *token = strtok(onlyTheNumbers, COMMA_STRING);
    int howMuchToIncreaseDC;
    int countNumbers=0;

    while (token != NULL)
    {
        /* check if there are really numbers in .dw, .db .dh */
        if (!isImmediate(token))
        {
            printf("[+] line %d: not a number in directive %s\n",lineNumber,guideword);
            return 1;
        }

        if (strcmp(guideword,DB) == 0)
            howMuchToIncreaseDC = DB_LENGTH;
        if (strcmp(guideword,DH) == 0)
            howMuchToIncreaseDC = DH_LENGTH;
        if (strcmp(guideword,DW) == 0)
            howMuchToIncreaseDC = DW_LENGTH;

        /* adding data to dataline list */
        addDataToDataLine(token, howMuchToIncreaseDC, howMuchToIncreaseDC);

        countNumbers++;
        token = strtok(NULL, COMMA_STRING);
    }

    if (countCommas(line) >= countNumbers){
        printf("[+] line %d: Error with commas in directive\n",lineNumber);
        return 1;
    }
    return 0;
}

/*
 * Get index of first char in string
 */
int getIndexOfFirst(char *s,char c)
{
    for (int i = 0; i < strlen(s) ; ++i)
    {
     if (s[i] == c)
         return i;
    }
    return -1;
}

/*
 * Get index of last char in string
 */
int getIndexOfLast(char *s,char c)
{
    int index = -1;
    for (int i = 0; i < strlen(s); i++)
        if (s[i] == c)
            index = i;
    return index;
}


/*
 * Parse the string in guide line of .asciz and send the data to addDataToDataLine function
 */
int handleAscizInGuidance(char* line,char* guideword)
{

    /* here the  guideword is .azciz */
    /* iterate over the string and save in data line */

    char *tempLine = (char*) malloc(strlen(line) * sizeof(char));
    strcpy(tempLine, line);
    char * onlyTheString = (char*) calloc(strlen(tempLine) , sizeof(char));
    int countOfOnlyTheString=0;

    /* get start and the end of the string */
    int first = getIndexOfFirst(tempLine,QUOTE);
    int last = getIndexOfLast(tempLine,QUOTE);

    /* Quote not found or just one quote found */
    if ((first == -1) || (last == -1) || (first == last))
    {
        printf("[+] line %d: illegal string <%s>\n", lineNumber, onlyTheString);
        return 1;
    }

    /* checking if there are bad characters before the quotes */
    for (int i = 0; i < first ; ++i)
    {
        if((tempLine[i] == SPACE) || (tempLine[i] == PERIOD) || (tempLine[i] == 'a') || (tempLine[i] == 's') || (tempLine[i] == 'c') || (tempLine[i] == 'i') || (tempLine[i] == 'z'))   /*space or .asciz */
            continue;
        else
        {
            printf("[+] line %d: illegal string <%s>\n", lineNumber, onlyTheString);
            return 1;
        }
    }

    /* checking if there are bad characters after the quotes */
    for (int i = last+1; i < strlen(tempLine) ; ++i)
    {
        if(tempLine[i] != SPACE)
        {
            printf("[+] line %d: illegal string <%s>\n", lineNumber, onlyTheString);
            return 1;
        }

    }

    /* skip the index of the quotes */
    first++;
    last--;

    /* build the actual string */
    while(first <= last)
    {
        onlyTheString[countOfOnlyTheString] = tempLine[first];
        countOfOnlyTheString++;
        first++;

    }

    /* add the string to data line */
    for (int i = 0; i < strlen(onlyTheString) ; ++i)
    {
        char * value = (char*) malloc(1 * sizeof(char));
        value[0] = onlyTheString[i];
        addDataToDataLine(value, 1, 0);
    }

    /* adding null byte at the end of the string */
    addDataToDataLine("\x00", 1, 0);
    return 0;
}

/* update DC according to data Line */
void updateDC(int ICF)
{
    DataLine *current = headData;
    while ( current != NULL) {
        current->DC = current->DC+ICF;
        current = current->next;
    }
}

/*
 * Parse the text in entry line
 */
char * getSymbolOfEntryLine(char* line,char* guideword)
{
    char *tempLine = (char*) malloc(strlen(line) * sizeof(char));
    strcpy(tempLine, line);
    removeSpaces(tempLine);
    char * onlyTheSymbol = (char*) malloc(strlen(tempLine) * sizeof(char));
    sliceString(tempLine,onlyTheSymbol,ENTRY_LENGTH, strlen(tempLine));
    return onlyTheSymbol;
}

/*
 * Writes the Data block to the .ob file
 */
void dataToObjectFile(FILE *fptr)
{
    DataLine *current = headData;
    char* buffer;
    char tempBuff[LENGTH_OF_BYTE];
    while ( current != NULL) {
        /* The data is a character - simply print it's ASCII value */
        if (current->type == 0)
        {
            if (current->DC % 4 == 0)
                fprintf(fptr, "\n%04d ", current->DC);

            fprintf(fptr, "%02X ", *(current->value));
        }
        /* The data is a byte - print the hex-value of it's two's complement binary representation */
        else if (current->type == 1)
        {
            if (current->DC % 4 == 0)
                fprintf(fptr, "\n%04d ", current->DC);

            buffer = (char*) malloc(sizeof(char) * LENGTH_OF_BYTE);
            convertTwosComplement(atoi(current->value), buffer, LENGTH_OF_BYTE);
            reverse(buffer, 0, 7);
            int num = hexify(buffer);
            fprintf(fptr, "%02X ", num);
            free(buffer);
        }
        /* The data is a half-word - print the hex-value of it's two's complement binary representation */
        else if (current->type == 2)
        {
            buffer = (char*) malloc(sizeof(char) * LENGTH_OF_HALF_WORD);
            convertTwosComplement(atoi(current->value), buffer, LENGTH_OF_HALF_WORD);

            for (int i = 0 ; i < 2 ; i++)
            {
                if ((current->DC+i) % 4 == 0)
                    fprintf(fptr, "\n%04d ", (current->DC+i));

                /*  copy 8 bits from the half-word, starting from the LSB, and hexify them for a couple of hex numbers*/
                memcpy(tempBuff, buffer + (LENGTH_OF_BYTE*i), LENGTH_OF_BYTE);
                reverse(tempBuff, 0, 7);
                int num = hexify(tempBuff);
                fprintf(fptr, "%02X ", num);
            }
            free(buffer);
        }
        /* The data is a word - print the hex-value of it's two's complement binary representation */
        else if (current->type == 4)
        {
            buffer = (char*) malloc(sizeof(char) * LENGTH_OF_WORD);
            convertTwosComplement(atoi(current->value), buffer, LENGTH_OF_WORD);

            for (int i = 0 ; i < 4 ; i++)
            {
                if ((current->DC+i) % 4 == 0)
                    fprintf(fptr, "\n%04d ", (current->DC+i));

                /*  copy 8 bits from the word, starting from the LSB, and hexify them for a couple of hex numbers*/
                memcpy(tempBuff, buffer + (LENGTH_OF_BYTE*i), LENGTH_OF_BYTE);
                reverse(tempBuff, 0, 7);
                int num = hexify(tempBuff);
                fprintf(fptr, "%02X ", num);
            }
            free(buffer);
        }

        current = current->next;
    }
}

/*
 * clear DataLine list after the first round
 */
void clearDataNodes()
{
    DataLine *current = headData;
    DataLine *next;
    while ( current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
    headData = NULL;
    globalDC = 0;
}