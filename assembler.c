/*
Parse arguments and read .as files
Include rest of functionality
*/

#include "assembler.h"

/* Global Data Block linked lists pointers */
SymbolNode *headSymbol = NULL;
SymbolNode *currentSymbol;

EntryNode *headEntry = NULL;
EntryNode *currentEntry;

int globalDC;
int globalIC;
int ICF;
int DCF;
int lineNumber;

/*
    * Handles the arguments and clears the data structs when done with each input file.
    * Return 0 if succeeded, otherwise returns 1 
	*/
int main(int argc, char* argv[])
{
    /* checking arguments */
    if ( (argc == 1) || (argc >= MAX_ARGUMENTS) )
    {
        fprintf(stderr, "[-] Error with arguments - need .as file and maximum 3 files.\n");
        exit(1);
    }

    FILE* fp;

    /* iterate over the input files */
    for (int i = 1; i < argc; ++i)
    {
        /* checking file extension*/
        if(strcmp(get_filename_ext(argv[i]),VALID_EXTENSION))
        {
            fprintf(stderr, "[-] Error with file extension (must be .as).\n");
            exit(1);
        }

        fp = fopen(argv[i], "r");
        if (fp != NULL)
        {
            assemble(fp, argv[i]);
            fclose(fp);
        }
        else
            printf("Error: Could not find file %s\n",argv[i]);

        /* clear global structs and variables for next file */
        clearEntryNodes();
        clearSymbolTable();
        clearDataNodes();
        clearInstructionNodes();
        clearLineNumber();
    }
    return 0;
}

/*
    * Handles a single file.
    * reads each line and calls parseLine
    * */
int assemble(FILE* fp, char* fileName)
{
    char *line = NULL;
    size_t len = MAX_LINE_LENGTH;
    unsigned long read = 0;
    int errorFlag = 0;

    while ((read = getline(&line, &len, fp)) != -1)
    {
        if (read > MAX_LINE_LENGTH)
        {
            printf("line %d: line length is above the maximum\n", lineNumber);
            errorFlag = 1;
            continue;
        }

        lineNumber++;

        /*Empty line*/
        if (read == 0)
            continue;

        int i = findFirstNonWhiteSpace(line);
        /* WhiteSpace\Comment line */
        if (i == len-1 || line[i] == ';')
            continue;

        trimNewLine(line, read);
        if(strcmp(line,EMPTY_STRING))
         {
            if (errorFlag == 1)
                parseLine(line);
            else
                errorFlag = parseLine(line);
         }

    }

    if (errorFlag == 1)
    {
        return 0;
    }

    /* Done with round 1 - now finalize ICF and DCF */
    ICF = globalIC;
    DCF = globalDC + ICF;
    globalDC = DCF;
    updateDC(ICF);
    updateSymbols(ICF);

    /* Start of round 2 */
    currentSymbol = headSymbol;
    currentEntry = headEntry;

    /* Add entry symbols */
    while (currentEntry != NULL)
    {
        errorFlag = 1;
        while (currentSymbol != NULL)
        {
            if(strcmp(currentEntry->data,currentSymbol->name) == 0)
            {
				/* Entry found in the symbol table */
                errorFlag = 0; 
                currentSymbol->attribute += 3;
                break;
            }
            currentSymbol = currentSymbol->next;
        }
        if (errorFlag == 1)
            printf("line %d: symbol <%s> does not exist\n", currentEntry->line, currentEntry->data);
        currentEntry = currentEntry->next;
        currentSymbol = headSymbol;
    }

    /* handling the names of output files */
    int indexOfPeriod = 0;
    char * extFilename = (char*) calloc(strlen(fileName)+1 , sizeof(char));

    /* checking if filename with full path */
    if( strrchr(fileName, '/') != NULL)
        {
        indexOfPeriod = getIndexOfLast(strrchr(fileName, '/')+1,PERIOD);
        sliceString(strrchr(fileName, '/')+1,extFilename,0,indexOfPeriod);
        }

    /* only file name was given */
    else
    {
        indexOfPeriod = getIndexOfLast(fileName,PERIOD);
        sliceString(fileName,extFilename,0,indexOfPeriod);
    }


    /* handling the names of output files */
    char * temp = (char*) calloc(strlen(fileName)+1 , sizeof(char));
    strcpy(temp,extFilename);
    strncat(extFilename, EXTERN_EXTENSION,LENGTH_EXTERN_EXTENSION);


    char * entFilename = (char*) calloc(strlen(fileName)+1 , sizeof(char));
    strcpy(entFilename,temp);
    strncat(entFilename, ENTRY_EXTENSION,LENGTH_ENTRY_EXTENSION);

    char * obFilename = (char*) calloc(strlen(fileName)+1 , sizeof(char));
    strcpy(obFilename,temp);
    strncat(obFilename, OBJECT_EXTENSION,LENGTH_OBJECT_EXTENSION);

    int fillError = labelFiller(extFilename);

    if (fillError == 0 && errorFlag == 0)
    {
        /* .ext file is written beforehand */
        if (checkForEntries())
            writeEntryFile(entFilename);
        else
            printf("[+] no .entry was found, .ent file won't be created\n");
        writeOBFile(obFilename);
    }
    else
    {
		/* remove .ext file and don't write anything else */
        remove(extFilename);
    }

   /* free memory */
    free(extFilename);
    free(entFilename);
    free(obFilename);
    return 1;
}


/*
 * This function parses the read line to seperate the label from the rest of the line
 * Return 0 if succeeded, otherwise returns non-zero value
 */
int parseLine(char *line)
{
	/* Get index of first word in the line */
    int startOfFirstWord = findFirstNonWhiteSpace(line);
    int lenOfFirstWord = findFirstWhiteSpace(line + startOfFirstWord);
    int errorFlag = 0;
    int parsingError;

	if (line[startOfFirstWord] == ':')
	{
		errorFlag = 1;
	}
    else if (line[startOfFirstWord+lenOfFirstWord-1] == ':')
    {
        /* Line contains a label */
        char *tempLine = (char*) malloc(strlen(line) * sizeof(char));
        strcpy(tempLine, line);

        char * theLabel = (char*) malloc(MAX_LABEL_LENGTH * sizeof(char));
        sliceString(tempLine,theLabel,startOfFirstWord, startOfFirstWord+lenOfFirstWord-1);

		/* invalid label */
        if (theLabel != NULL && checkLabel(theLabel))
            errorFlag = 1;

        parsingError = parseLine2(line+startOfFirstWord+lenOfFirstWord, theLabel);
    }
    else
        parsingError = parseLine2(line,NULL);

    if (errorFlag == 0)
        return parsingError;
    return errorFlag;
}

/*
 * This function parses the command\directive portion of the line
 * Return 0 if succeeded, otherwise returns non-zero value
 */
int parseLine2(char *line, char *theLabel)
{
    int startOfFirstWord = findFirstNonWhiteSpace(line);
    int lenOfFirstWord = findFirstWhiteSpace(line + startOfFirstWord);
    char *instName;
    int errorFlag = 0;
	
	if (line[startOfFirstWord] == ';')
	{
		printf("line %d: label <%s> followed by comment\n",lineNumber, theLabel);
		return 1;	
	}
    else if (line[startOfFirstWord] == '.')
    {
		/* get only the guide word from line (.dw,.entry...) */
		char guideWord[EXTERN_LENGTH];
        sliceString(&(line[startOfFirstWord]),guideWord,0,findFirstWhiteSpace(&(line[startOfFirstWord])));
        /* check if the word after the . is ok,print an error if not */
        if(!checkGuideword(guideWord))
        {
            printf("line %d: unrecognized directive <%s>\n",lineNumber, guideWord);
            errorFlag = 1;
        }

        /* guideword is ok */
        if (strcmp(guideWord,DB)==0 || strcmp(guideWord,DW)==0 ||strcmp(guideWord,DH)==0 )
        {

            if (theLabel != NULL)
            {
                /* getting the correct attribute for symbol table */
                int attribute = getAttributeForGuide(guideWord);

                /* add to symbol table because it was a line with label */
                errorFlag += addSymbol(theLabel, globalDC, attribute);
            }

           errorFlag += handleNumbersInGuidance((&(line[startOfFirstWord])), guideWord);
        }

        /* guideword is ok */
        else if (strcmp(guideWord,ASCIZ)==0)
        {
            if (theLabel != NULL)
            {
                /* getting the correct attribute for symbol table */
                int attribute = getAttributeForGuide(guideWord);

                /* add to symbol table because it was a line with label */
                errorFlag += addSymbol(theLabel, globalDC, attribute);
            }
			
            errorFlag += handleAscizInGuidance((&(line[startOfFirstWord])), guideWord);
        }

        /* if guideword is .extern add label to symbol table with value zero */
        else if (strcmp(guideWord,EXTERN)==0)
        {
            /* getting the correct attribute for symbol table */
            int attribute = getAttributeForGuide(guideWord);
            char * symbolOfExtern = getSymbolOfExternLine((&(line[startOfFirstWord])), guideWord);
            /* add to symbol table because it was a line with label */
            errorFlag += addSymbol(symbolOfExtern, 0, attribute);
        }

        /* if guideword is .entry print warning (p.17 in mamans book) */
        else if (strcmp(guideWord,ENTRY)==0 && theLabel != NULL)
        {
            printf("[*] Warning - a label and than .entry word");
        }

        else if (strcmp(guideWord,ENTRY)==0)
        {
            errorFlag += addLabelToEntryNode(getSymbolOfEntryLine((&(line[startOfFirstWord])), guideWord));
        }

    }
    else
    {
        /* Command instruction*/
        if (theLabel != NULL)
        {
            /* add to symbol table because it was a line with label */
            errorFlag = addSymbol(theLabel, globalIC, 0);
        }

        instName = (char*)calloc(lenOfFirstWord+1, sizeof(char));
        strncpy(instName, line + startOfFirstWord, lenOfFirstWord);

        if (errorFlag == 1)
            handleInstruction(instName, line);
        else
            errorFlag += handleInstruction(instName, line);
        free(instName);
    }
    return errorFlag;
}

/*
 * This function adds a label to the symbol table
 * Return 0 if succeeded, otherwise returns non-zero value
 */
int addSymbol(char *name, int value, int attribute)
{
    int count = 0;
    
	/* initialize first node */
    if (headSymbol == NULL)
    {
        headSymbol = (struct SymbolNode*)malloc(sizeof(struct SymbolNode));
        currentSymbol = headSymbol;
        currentSymbol->name = name;
        currentSymbol->value = value;
        currentSymbol->attribute = attribute;
        currentSymbol->next = NULL;
    }
    else
    {
        SymbolNode *temp = headSymbol;
        while (temp != NULL)
        {
            if (strcmp(temp->name, name) == 0)
            {
                printf("lind %d: symbol <%s> already exists\n", lineNumber, name);
                return 1;
            }
            temp = temp->next;
        }

        struct SymbolNode* newNode = (struct SymbolNode*)malloc(sizeof(struct SymbolNode));
        newNode->value = value;
        newNode->next = NULL;
        newNode->attribute = attribute;
        newNode->name = name;
        currentSymbol->next = newNode;
        currentSymbol = newNode;
    }

    return 0;
}

/*
 * This function updates the DC value of the symbol table by the ICF
 */
void updateSymbols(){
    SymbolNode *current = headSymbol;
    while ( current != NULL) {
        if (current->attribute == 1)
            current->value = current->value + ICF;
        current = current->next;
    }
}

/*
 * This function adds an .entry directive to a linked-list of .entry directives for the .ent file
 */
int addLabelToEntryNode(char*  theLabel)
{
	/* initialize first node */
    if (headEntry == NULL)
    {
        headEntry = (struct EntryNode*)malloc(sizeof(struct EntryNode));
        currentEntry = headEntry;
        currentEntry->data = theLabel;
        currentEntry->line = lineNumber;
        currentEntry->next = NULL;
    }
    else
    {
        EntryNode *temp = headEntry;
        while (temp != NULL)
        {
            if (strcmp(temp->data, theLabel) == 0)
            {
                printf("line %d: symbol <%s> already exists\n", lineNumber, theLabel);
                return 1;
            }
            temp = temp->next;
        }


        struct EntryNode* newNode = (struct EntryNode*)malloc(sizeof(struct EntryNode));
        newNode->data = theLabel;
        newNode->line = lineNumber;
        newNode->next = NULL;
        currentEntry->next = newNode;
        currentEntry = newNode;
    }

    return 0;
}

/*
 * This function fills the missing binary encodings of labels in instructions from the 1st round
 */
int labelFiller(char* name)
{
    InstructionLine *current = headInstruction;
    currentSymbol = headSymbol;
    int foundSymbol = 0;
    int errorFlag = 0;
    int i = 0;
    int immed;
    char *fill = NULL;
    FILE *fptr = fopen(name,"w"); /* open .ext file */
    int extFlag = 0;
    int written = 0;

	/* loop over the instruction block to check if instrcution is missing a label binary encoding */
    while ( current != NULL)
    {
        if(current->label != NULL)
        {
            /* loop over symbol table to find missing label */
            while ( currentSymbol != NULL)
            {
                if(strcmp(currentSymbol->name,current->label) == 0)
                {
                    foundSymbol = 1;
					/* The instruction is I-type, calculate offset from address */
                    if ((strcmp(current->type, "I")) == 0)
                    {
						/* Check if symbol is .external - can't happen in branch */
                        if(currentSymbol->attribute == 2)
                        {
                            printf("line %d: Required symbol for branch can't be external <%s>\n",current->line,currentSymbol->name);
                            errorFlag++;
                        }
                        else
                        {
                            immed = currentSymbol->value - current->IC;
                            fill = (char*) malloc(sizeof(char) * LENGTH_OF_HALF_WORD);
                            convertTwosComplement(immed, fill, LENGTH_OF_HALF_WORD);
                        }
                    }
					/* The instruction is J-type, encode address */
                    else if ((strcmp(current->type, "J")) == 0)
                    {
                        fill = (char*) malloc(sizeof(char) * LENGTH_OF_ADDRESS);
						
						/* Check if symbol is .external - the address is 0's*/
                        if(currentSymbol->attribute == 2)
                        {
                            fprintf(fptr,"%s %d\n",currentSymbol->name, current->IC);
                            written = 1;
                            extFlag = 1;
                            for (int j = 0 ; j < 25 ; j++)
                                fill[j] = ZERO_CHAR;
                        }
                        else
                            itoa(currentSymbol->value, fill, 2, LENGTH_OF_ADDRESS);
                    }
					
					/* If it was found there is a need to fill the label - replace the 'X' with the real encoding */
                    if (fill != NULL)
                    {
                        while(current->value[i] ==X_CHAR)
                        {
                            current->value[i] = fill[i];
                            i++;
                        }
                        i = 0;
                        fill = NULL;
                    }
                }
                currentSymbol = currentSymbol->next;
            }
            if(!foundSymbol)
            {
                printf("[+] line %d: Symbol not found <%s>\n",current->line,current->label);
                errorFlag++;
            }
            else
                foundSymbol = 0;

        }
        currentSymbol = headSymbol;
        current = current->next;
    }
    fclose(fptr);
    if(written == 0)
        remove(name);

    return errorFlag;
}

/*
 * This function writes the .ent file of entries which were used in the file
 */
void writeEntryFile(char *file)
{
    FILE *fptr;
    fptr = fopen(file,"w");
    SymbolNode *current = headSymbol;
    while ( current != NULL)
    {
        if(current->attribute > 2)
            fprintf(fptr,"%s %d\n",current->name,current->value);
        current = current->next;

    }
    fclose(fptr);
}

/*
 * This function checks if any symbol is of .entry type
 */
int checkForEntries()
{
    SymbolNode *current = headSymbol;
    while ( current != NULL)
    {
        if(current->attribute > 2)
            return 1;
        current = current->next;
    }
    return 0;
}

/*
 * This function creates and writes the .ob file
 */
void writeOBFile(char *file)
{
    FILE *fptr;
    fptr = fopen(file,"w");
    instructionsToObjectFile(fptr, ICF, DCF);
    dataToObjectFile(fptr);
    fclose(fptr);
}

/*
 * This function gets the extension of a filename
 */
char *get_filename_ext(const char *filename) {
    char *dot = strrchr(filename, PERIOD);
    if(!dot || dot == filename) return EMPTY_STRING;
    return dot + 1;
}

/*
 * This function clears the data struct of entries for the next input file
 */
void clearEntryNodes(){
    EntryNode *current = headEntry;
    EntryNode *next;
    while ( current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
    headEntry = NULL;
}

/*
 * This function clears the data struct of the symbol table for the next input file
 */
void clearSymbolTable(){
    SymbolNode *current = headSymbol;
    SymbolNode *next;
    while ( current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
    headSymbol = NULL;
}