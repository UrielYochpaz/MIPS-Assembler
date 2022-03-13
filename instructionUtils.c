/*
 * This file contains functions that handles command instruction
 */
 
#include "instructionUtils.h"

/* Global linked list struct of Data Block */
InstructionLine *headInstruction = NULL;
InstructionLine *currentInstruction;
int globalIC = START_IC;
int lineNumber;

/*
 * The main function which handles the instrcution line
 */
int handleInstruction(char *instructionName, char* line)
{
    Command *metaCommand = checkInstruction(instructionName, line);

    if (metaCommand == NULL)
    {
        printf("line %d: unknown instruction <%s>\n", lineNumber, instructionName);
        return 1;
    }

    int operatorsStatus = checkOperators(instructionName, line, metaCommand);
    if (operatorsStatus == 0) {
        return 1;
    }

    return 0;
}

/*
 * Validates the operation is real, and returns it's relevant struct
 * containing the instruciotn's type, opcode and funct
*/
Command* checkInstruction(char *instructionName, char* line)
{
    for (int i = 0; i < NUM_OF_COMMS; i++)
    {
        if (strcmp(instructionName, validInstructions[i].name) == 0)
            return &validInstructions[i];
    }
    return NULL;
}

/*
 * Parses the line's operators according to the instrcution's operation
*/
int checkOperators(char* instructionName, char* line, Command *metaCommand)
{
    char *tempLine = (char*) malloc(strlen(line) * sizeof(char));
    strcpy(tempLine, line);
    removeSpaces(tempLine);
    char * operators = (char*) malloc(strlen(tempLine) * sizeof(char));
    sliceString(tempLine,operators,strlen(instructionName), strlen(tempLine));

    int regNum;
    int count = 0;

    if (metaCommand->opcode == 0)
    {
        /* R-Type arithmetic\logic operation*/
        char registers[3][5];
        char *token = strtok(operators, COMMA_STRING);
        while (token != NULL)
        {
            if(++count > 3)
            {
                printf("line %d: too many operands\n", lineNumber);
                return 0;
            }

            regNum = isRegister(token, 1);
            if (regNum == -1)
            {
                /* not a register */
                return 0;
            }

            itoa(regNum, registers[count-1], 2, 5);
            token = strtok(NULL, COMMA_STRING);
        }
        if (count < 3)
        {
            printf("line %d: not enough operands\n", lineNumber);
            return 0;
        }
        if (countCommas(line) >= count)
        {
            printf("line %d: Error with commas in instruction\n",lineNumber);
            return 0;
        }
        addInstructionToInstructionLine(metaCommand, 5, registers, NULL, NULL);
        return 1;
    }

    else if (metaCommand->opcode == 1)
    {
        /* R-Type copy operation */
        char registers[2][5];
        char *token = strtok(operators, COMMA_STRING);
        while (token != NULL)
        {
            if(++count > 2)
            {
                printf("line %d: too many operands\n", lineNumber);
                return 0;
            }

            regNum = isRegister(token, 1);
            if (regNum == -1)
            {
                /* not a register*/
                return 0;
            }

            itoa(regNum, registers[count-1], 2, 5);
            token = strtok(NULL, COMMA_STRING);
        }
        if (count < 2)
        {
            printf("line %d: not enough operands\n", lineNumber);
            return 0;
        }
        if (countCommas(line) >= count)
        {
            printf("line %d: Error with commas in instruction\n",lineNumber);
            return 0;
        }
        addInstructionToInstructionLine(metaCommand, 5, registers, NULL, NULL);
        return 1;
    }

    else if (metaCommand->opcode >= 10 && metaCommand->opcode <= 14)
    {
        /* I-Type arithmetic\logic operation */
        char registers[2][5];
        char immedAsBinary[LENGTH_OF_HALF_WORD];
        char *token = strtok(operators, COMMA_STRING);
        while (token != NULL)
        {
            if(++count > 3)
            {
                printf("line %d: too many operands\n", lineNumber);
                return 0;
            }

            if (count ==1 || count ==3)
            {
                regNum = isRegister(token, 1);
                if (regNum == -1)
                {
                    /* not a register */
                    return 0;
                }
                if(count == 1)
                {
                    itoa(regNum, registers[0], 2, 5);
                }
                if(count == 3)
                {
                    itoa(regNum, registers[1], 2, 5);
                }
            }

            if (count == 2)
            {
                if (!isImmediate(token))
                {
                    /* Not an immediate value */
                    return 0;
                }

                itoa(atoi(token), immedAsBinary, 2, LENGTH_OF_HALF_WORD);
                convertTwosComplement(atoi(token), immedAsBinary, LENGTH_OF_HALF_WORD);

            }
            token = strtok(NULL, COMMA_STRING);
        }
        if (count < 3)
        {
            printf("line %d: not enough operands\n", lineNumber);
            return 0;
        }
        if (countCommas(line) >= count)
        {
            printf("line %d: Error with commas in instruction\n",lineNumber);
            return 0;
        }
        addInstructionToInstructionLine(metaCommand, 5, registers, immedAsBinary, NULL);
        return 1;

    }

    else if (metaCommand->opcode >= 15 && metaCommand->opcode <= 18)
    {
        /* I-Type branch operation */

        char registers[2][5];
        char immedAsBinary[LENGTH_OF_HALF_WORD];
        char *token = strtok(operators, COMMA_STRING);
        char *label = NULL;

        while (token != NULL)
        {
            if(++count > 3)
            {
                printf("line %d: too many operands\n", lineNumber);
                return 0;
            }

            if ((count ==1) || (count ==2))
            {
                regNum = isRegister(token, 1);
                if (regNum == -1)
                {
                    /* not a register */
                    return 0;
                }
                if (count == 1)
                {
                    itoa(regNum, registers[0], 2, 5);
                }
                if (count == 2)
                {
                    itoa(regNum, registers[1], 2, 5);
                }
            }

            if (count == 3)
            {
                if (checkLabel(token))
                {
                    /* Not a legit label */
                    return 0;
                }

                label = token;
                /*
				 * the immediate value in the binary encoding is filles with 'X's,
				 * and will be replaced by the real value at the second round
				 */
                for (int i = 0 ; i <= 15 ; i++)
                    immedAsBinary[i] = X_CHAR;
            }
            token = strtok(NULL, COMMA_STRING);

        }
        if (count < 3)
        {
            printf("line %d: not enough operands\n", lineNumber);
            return 0;
        }
        if (countCommas(line) >= count)
        {
            printf("line %d: Error with commas in instruction\n",lineNumber);
            return 0;
        }
        addInstructionToInstructionLine(metaCommand, 5, registers, immedAsBinary, label);
        return 1;
    }

    else if (metaCommand->opcode >= 19 && metaCommand->opcode <= 24)
    {
        /* I-Type load\store operation */
        char registers[2][5];
        char immedAsBinary[LENGTH_OF_HALF_WORD];
        char *token = strtok(operators, COMMA_STRING);
        while (token != NULL)
        {
            if(++count > 3)
            {
                printf("line %d: too many operands\n", lineNumber);
                return 0;
            }

            if ((count ==1) || (count ==3))
            {
                regNum = isRegister(token, 1);
                if (regNum == -1)
                {
                    /* not a register */
                    return 0;
                }
                if (count == 1)
                {
                    itoa(regNum, registers[0], 2, 5);
                }
                if (count == 3)
                {
                    itoa(regNum, registers[1], 2, 5);
                }
            }

            if (count == 2)
            {
                if (!isImmediate(token))
                {
                    /* Not an immediate value */
                    return 0;
                }

                itoa(atoi(token), immedAsBinary, 2, LENGTH_OF_HALF_WORD);
                convertTwosComplement(atoi(token), immedAsBinary, LENGTH_OF_HALF_WORD);

             }
            token = strtok(NULL, COMMA_STRING);

        }
        if (count < 3)
        {
            printf("line %d: not enough operands\n", lineNumber);
            return 0;
        }
        if (countCommas(line) >= count)
        {
            printf("line %d: Error with commas in instruction\n",lineNumber);
            return 0;
        }
        addInstructionToInstructionLine(metaCommand, 5, registers, immedAsBinary, NULL);
        return 1;
    }

    else if (metaCommand->opcode == 30)
    {
        /* J-Type jmp operation */
        int flagReg = 0; /* 0 - label, 1 - register */
        char registers[1][LENGTH_OF_ADDRESS];
        char reg;
        char *label = NULL;

        if (countCommas(line) > 0)
        {
            printf("line %d: Error with commas in instruction\n",lineNumber);
            return 0;
        }

        regNum = isRegister(operators, 0);
        if (regNum == -1)
        {
            /* Not a register - check if a legit label */
            if (checkLabel(operators))
            {
                /* Not a legit label */
                return 0;
            }
            label = operators;
            /*
			 * the address value in the binary encoding is filles with 'X's,
			 * and will be replaced by the real value at the second round
			 */
            for (int i = 0 ; i < LENGTH_OF_ADDRESS ; i++)
                registers[0][i] = X_CHAR;
        }
        else
        {
			/* The operand is a register, binary encoding happens now*/
            flagReg = 1;
            itoa(regNum, registers[0], 2, LENGTH_OF_ADDRESS);
        }
        itoa(flagReg, &reg, 10, 1);
        addInstructionToInstructionLine(metaCommand, 1, registers, &reg, label);
        return 1;
    }

    else if (metaCommand->opcode == 31 || metaCommand->opcode == 32)
    {
        /* J-Type la operation */
        char registers[1][LENGTH_OF_ADDRESS];
        char reg = ZERO_CHAR;
        char *label = NULL;

        if (countCommas(line) > 0)
        {
            printf("line %d: Error with commas in instruction\n",lineNumber);
            return 0;
        }

        if (checkLabel(operators))
        {
            /* Not a legit label */
            return 0;
        }
        else
        {
            label = operators;
			/*
			 * the address value in the binary encoding is filles with 'X's,
			 * and will be replaced by the real value at the second round
			 */
            for (int i = 0 ; i < LENGTH_OF_ADDRESS ; i++)
                registers[0][i] = X_CHAR;
        }
        addInstructionToInstructionLine(metaCommand, 1, registers, &reg, label);
        return 1;
    }

    else if (metaCommand->opcode == 63)
    {
        /* J-Type stop operation */
        char registers[1][LENGTH_OF_ADDRESS];
        char reg = ZERO_CHAR;
        if (strcmp(operators, EMPTY_STRING))
        {
            printf("line %d: too many operands\n", lineNumber);
            return 0;
        }
        else
        {
			/*
			 * the address value in the binary encoding is filles with 'X's,
			 * and will be replaced by the real value at the second round
			 */
            for (int i = 0 ; i < LENGTH_OF_ADDRESS ; i++)
                registers[0][i] = ZERO_CHAR;
        }
        addInstructionToInstructionLine(metaCommand, 1, registers, &reg, NULL);
        return 1;
    }

    else
    {
        /* In the weird case no condition matches */
        return 0;
    }
}

/*
 * Receives a pointer to an operator and checks if it's a register
 * Returns the register's number if it's a legit register, else it returns -1
 */
int isRegister(char *token, int errorPrint)
{
    int regNum;

    if (token[0] != '$')
    {
        if (errorPrint == 1)
            printf("line %d: the operand <%s> is invalid (must be a register)\n", lineNumber, token);
        return -1;
    }

    if (strlen(token) == 3 && isdigit(token[1]) && isdigit(token[2]))
    {
        regNum = atoi(token+1);
        if (regNum >= 0 && regNum < 32)
            return regNum;
        else
        {
            if (errorPrint == 1)
                printf("line %d: the operand <%s> is invalid (register in range of 0-31)\n", lineNumber, token);
            return -1;
        }
    }
    else if (strlen(token) == 2 && isdigit(token[1]))
    {
        regNum = atoi(token+1);
        return regNum;
    }
    else
    {
        if (errorPrint == 1)
            printf("line %d: the operand <%s> is invalid (must be a register)\n", lineNumber, token);
        return -1;
    }
}

/*
 * Creates a node in the linked-list representing the Instruction Block
 */
void addInstructionToInstructionLine(Command *metaCommand, size_t m, char registers[][m], char *immede, char *label)
{
    if (headInstruction == NULL)
    {
        headInstruction = (struct InstructionLine*)malloc(sizeof(struct InstructionLine));
        headInstruction->IC = globalIC;
        globalIC += 4;
        headInstruction->line = lineNumber;
        strcpy(headInstruction->type, metaCommand->type);
        headInstruction->label = label;
        headInstruction->next = NULL;
        currentInstruction = headInstruction;
    }
    else
    {
        struct InstructionLine * newNode = (struct InstructionLine*)malloc(sizeof(struct InstructionLine));
        newNode->IC = globalIC;
        globalIC += 4;
        newNode->line = lineNumber;
        strcpy(newNode->type, metaCommand->type);
        newNode->label = label;
        newNode->next = NULL;
        currentInstruction->next = newNode;
        currentInstruction = newNode;
    }

    fillValue(metaCommand, m, registers, immede, currentInstruction);
}

/*
 * Fills the value member of the linked-list node (the binary encoding) 
 */
void fillValue(Command *metaCommand, size_t m, char registers[][m], char *immede, InstructionLine *node)
{
    char tempValue[COMMAND_SIZE];
    char *temp;
    if ( metaCommand->opcode == 0 )
    {
        /* 0-5 - reserved bits */
        for (int i = 0 ; i < 6 ; i++)
            tempValue[i] = ZERO_CHAR;

        /* allocate a 6-byte string for funct and registers (includes null-terminated byte) */
        temp = (char*) malloc(6 * sizeof(char));

        /* funct */
        itoa(metaCommand->funct, temp, 2, 5);
        strcpy(tempValue+6, temp);

        /* rd register */
        strcpy(tempValue+11, registers[2]);

        /* rt register */
        strcpy(tempValue+16, registers[1]);

        /* rs register */
        strcpy(tempValue+21, registers[0]);

        /* allocate a 7-byte string for opcode (includes null-terminated byte) */
        free(temp);
        temp = (char*) malloc(7 * sizeof(char));
        
		/* opcode */
        itoa(metaCommand->opcode, temp, 2, 6);
        strcpy(tempValue+26, temp);

        /* Copy to node->value */
        memcpy(node->value, tempValue, LENGTH_OF_WORD);
    }
    else if ( metaCommand->opcode == 1 )
    {
        /* 0-5 - reserved bits */
        for (int i = 0 ; i < 6 ; i++)
            tempValue[i] = ZERO_CHAR;

        /* allocate a 6-byte string for funct and registers (includes null-terminated byte) */
        temp = (char*) malloc(6 * sizeof(char));

        /* funct */
        itoa(metaCommand->funct, temp, 2, 5);
        strcpy(tempValue+6, temp);

        /* rd register */
        strcpy(tempValue+11, registers[1]);

        /* rt register - 0'd */
        for (int i = 0 ; i < 5 ; i++)
            tempValue[i+16] = ZERO_CHAR;

        /* rs register */
        strcpy(tempValue+21, registers[0]);

        /* allocate a 7-byte string for opcode (includes null-terminated byte) */
        free(temp);
        temp = (char*) malloc(7 * sizeof(char));
        
		/* opcode */
        itoa(metaCommand->opcode, temp, 2, 6);
        strcpy(tempValue+26, temp);

        /* Copy to node->value */
        memcpy(node->value, tempValue, LENGTH_OF_WORD);
    }
    if (metaCommand->opcode >= 10 && metaCommand->opcode <= 14)
    {
        /* immed */
        strcpy(tempValue, immede);

        /* rt register */
        strcpy(tempValue+16, registers[1]);

        /* rs register */
        strcpy(tempValue+21, registers[0]);

        /* allocate a 7-byte string for opcode (includes null-terminated byte) */
        temp = (char*) malloc(7 * sizeof(char));
        
		/* opcode */
        itoa(metaCommand->opcode, temp, 2, 6);
        strcpy(tempValue+26, temp);

        /* Copy to node->value */
        memcpy(node->value, tempValue, LENGTH_OF_WORD);
    }
    else if (metaCommand->opcode >= 15 && metaCommand->opcode <= 24 )
    {
        /* immed */
        strcpy(tempValue, immede);

        /* rt register */
        strcpy(tempValue+16, registers[1]);

        /* rs register */
        strcpy(tempValue+21, registers[0]);

        /* allocate a 7-byte string for opcode (includes null-terminated byte) */
        temp = (char*) malloc(7 * sizeof(char));
        
		/* opcode */
        itoa(metaCommand->opcode, temp, 2, 6);
        strcpy(tempValue+26, temp);

        /* Copy to node->value */
        memcpy(node->value, tempValue, LENGTH_OF_WORD);
    }
    else if (metaCommand->opcode >= 30 && metaCommand->opcode <= 32 || metaCommand->opcode == 63)
    {
        /* address */
        strcpy(tempValue, registers[0]);

        /* reg */
        strcpy(tempValue+25, immede);

        /* allocate a 7-byte string for opcode (includes null-terminated byte) */
        temp = (char*) malloc(7 * sizeof(char));
        itoa(metaCommand->opcode, temp, 2, 6);
        strcpy(tempValue+26, temp);

        /* Copy to node->value */
        memcpy(node->value, tempValue, LENGTH_OF_WORD);
    }
}

/*
 * Writes the Instructions to the .ob file
 */
void instructionsToObjectFile(FILE *fptr, int icf, int dcf)
{
	/* Write the Instruction and Data blocks lengths to the top of the file */
    fprintf(fptr, "     %d %d", icf-START_IC, dcf-icf);
    InstructionLine *current = headInstruction;
    while (current != NULL) {
		/* the value is reversed - first bit (value[0])is LSB, the last bit are MSB */
        char *first = current->value;
        reverse(first, 0, 7);
        int num1 = hexify(first);

        char *second = current->value+LENGTH_OF_BYTE;
        reverse(second, 0, 7);
        int num2 = hexify(second);

        char *third = current->value+16;
        reverse(third, 0, 7);
        int num3 = hexify(third);

        char *fourth = current->value+24;
        reverse(fourth, 0, 7);
        int num4 = hexify(fourth);

        fprintf(fptr, "\n%04d %02X %02X %02X %02X", current->IC, num1, num2, num3, num4);

        current = current->next;
    }
}

/*
 * Clear the whole list and initialize the starting IC for more input files
 */
void clearInstructionNodes(){
    InstructionLine *current = headInstruction;
    InstructionLine *next;
    while ( current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }
    headInstruction = NULL;
    globalIC = START_IC;
}