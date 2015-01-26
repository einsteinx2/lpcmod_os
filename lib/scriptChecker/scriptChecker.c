/*
 * scriptChecker.c
 *
 *  Created on: Jan 16, 2015
 *      Author: bennydiamond
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned short u16;

#define ARGTYPE_UNKNOWN -1
#define ARGTYPE_FUNCTION 0
#define ARGTYPE_OPERATOR 1
#define ARGTYPE_VARIABLE 2
#define ARGTYPE_NUMERIC  3

#define OPTYPE_EQ_EQ    0
#define OPTYPE_NOT_EQ   1
#define OPTYPE_GREATER  2
#define OPTYPE_LOWER    3
#define OPTYPE_GREATER_EQ  4
#define OPTYPE_LOWER_EQ    5
#define OPTYPE_EQ       6
#define OPTYPE_PLUS     7
#define OPTYPE_MINUS    8
#define OPTYPE_MULT     9
#define OPTYPE_DIVIDE   10
#define OPTYPE_MOD      11

#define FUNCTION_IF     0
#define FUNCTION_ELSE   1
#define FUNCTION_ENDIF  2
#define FUNCTION_GOTO   3
#define FUNCTION_GPI    4
#define FUNCTION_GPO    5
#define FUNCTION_WAIT   6
#define FUNCTION_BOOT   7
#define FUNCTION_FAN    8
#define FUNCTION_LED    9
#define FUNCTION_LCDW   10
#define FUNCTION_LCDC   11
#define FUNCTION_LCDR   12
#define FUNCTION_LCDB   13
#define FUNCTION_LCDP   14
#define FUNCTION_VAR    15
#define FUNCTION_END    16

#define NBFUNCTIONCALLS 17

#define BNKFULLTSOP   0x00u
#define BNKTSOPSPLIT0 0x10u
#define BNKTSOPSPLIT1 0x18u
#define BNK512  0x84u
#define BNK256  0x86u
#define BNKOS  0x87u


typedef struct variableEntry{
    struct variableEntry * previous;
    struct variableEntry * next;
    int value;
    char * name;
}variableEntry;

typedef struct _variableList{
    int count;
    variableEntry * first;
    variableEntry * last;
}_variableList;

typedef struct labelEntry{
    struct labelEntry * previous;
    struct labelEntry * next;
    int stringPos;
    char * name;
}labelEntry;

typedef struct _labelList{
    int count;
    labelEntry * first;
    labelEntry * last;
}_labelList;

typedef struct ifStatementEntry{
    int ifPosition;
    int elsePosition;
    int endifPosition;
    struct ifStatementEntry *next;
    struct ifStatementEntry *previous;
}ifStatementEntry;

typedef struct _ifStatementList{
    int count;
    ifStatementEntry *first;
    ifStatementEntry *last;
}_ifStatementList;

bool stringDeclaration;

void runScript(u8 * file, u32 fileSize, int paramCount, int * param);

bool ifFunction(int param1, u8 op, int param2);
bool gpiFunction(u8 port);
bool gpoFunction(u8 port, u8 value);
bool waitFunction(int ms);
bool bootFunction(u8 bank);
bool fanFunction(u8 value);
bool ledFunction(char * value);
bool lcdPrintFunction(u8 line, char * text);
bool lcdClearLineFunction(u8 line);
bool lcdResetFunction(void);
bool lcdBacklightFunction(u8 value);
bool lcdPowerFunction(u8 value);
bool variableFunction(char * name, int initValue, _variableList * variableList);
bool checkEndOfArgument(char * compareBuf, int position);
bool updateVariable(char * name, int value, _variableList * variableList);


char * functionCallList[NBFUNCTIONCALLS] = {
        "IF",
        "ELSE",
        "ENDIF",
        "GOTO",
        "GPI",
        "GPO",
        "WAIT",
        "BOOT",
        "FAN",
        "LED",
        "LCDW",
        "LCDC",
        "LCDR",
        "LCDB",
        "LCDP",
        "VAR",
        "END"
};


void parseFileForLabels(u8 * file, u32 fileSize, _labelList * labelList);
void addNewLabel(struct _labelList * labelList, char * compareBuf, int position);
bool parseFileForIFStatements(u8 * file, u32 fileSize, _ifStatementList * ifStatementList);
int decodeArgument(char * inputArg, int * outNum, char ** string, _variableList * variableList);

void showUsage(const char *progname);

int main (int argc, const char * argv[])
{
	FILE *pFile;
	u32 fileSize;
	u8 * buffer;
    printf("XBlast OS script syntax checker.\n");

    if( argc < 2 ) {
        showUsage(argv[0]);
        exit(1);
    }

    pFile = fopen(argv[1],"rb");
    if(pFile == NULL){
        printf("File open error.\n");
        exit(1);
    }

    fseek(pFile, 0, SEEK_END);
    fileSize = ftell(pFile);
    if(fileSize == -1)
    {
    	printf("File size error...");
    	exit(1);
    }
    fseek(pFile, 0, SEEK_SET);
    buffer = (u8 *)malloc(sizeof(u8) * (fileSize + 1));

    fileSize = fread(buffer, sizeof(u8), fileSize, pFile);

    fclose(pFile);
    runScript(buffer, fileSize, 0, NULL);

    free(buffer);

	return EXIT_SUCCESS;
}


void showUsage(const char *progname) {
    printf("Usage:   ");
    printf("%s 'input file'\n",progname);
}

void runScript(u8 * file, u32 fileSize, int paramCount, int * param){
    int insideIfStatement = 0;
    int i, j;
    int arithAccumulator;
    bool accumulatorInUse;
    int arithOpInLine;
    _labelList labelList;
    labelEntry * labelEntry;

    _variableList variableList;
    variableEntry * currentEntry;

    _ifStatementList ifStatementList;
    ifStatementEntry * ifEntry;


    int stringStartPtr = 0, stringStopPtr = 0, tempPtr;
    int argStartPtr[5], argEndPtr[5];
    struct{
        bool exist;
        u8 type;
        int value;
        char * text;
    }argumentList[5];
    int nbArguments;
    bool CRdetected;
    char compareBuf[100];                     //100 character long seems acceptable
    char tempBuf[50];

    stringDeclaration = false;

    variableList.count = 0;
    variableList.first = NULL;
    variableList.last = NULL;
    //Add parameters passed in runScript function call as variables.
    for(i = 0; i < paramCount; i++){
        sprintf(tempBuf, "_param%u", i);
        variableFunction(tempBuf, param[i], &variableList);
    }
    variableFunction("BNK512", BNK512, &variableList);
    variableFunction("BNK256", BNK256, &variableList);
    variableFunction("BNKOS", BNKOS, &variableList);
    variableFunction("BNKFULLTSOP", BNKFULLTSOP, &variableList);
    variableFunction("BNKTSOPSPLIT0", BNKTSOPSPLIT0, &variableList);
    variableFunction("BNKTSOPSPLIT1", BNKTSOPSPLIT1, &variableList);

    labelList.count = 0;
    labelList.first = NULL;
    labelList.last = NULL;
    ifStatementList.count = 0;
    ifStatementList.first = NULL;
    ifStatementList.last = NULL;

    //Parse for labels first
    //Put all found in labelList
    parseFileForLabels(file, fileSize, &labelList);
    printf("\n\nTotal labels found : %u", labelList.count);

    if(!parseFileForIFStatements(file, fileSize, &ifStatementList)){
        printf("\n\nError in IF/ELSE/ENDIF statement logic!!!!");
    	goto endExecution;
    }
    printf("\n\nTotal IF/ELSE/ENDIF statements found : %u", ifStatementList.count);
    printf("\n\nBegin script execution");

    while(stringStopPtr < fileSize){      //We stay in file
        while(file[stringStopPtr] != '\n' && stringStopPtr < fileSize){        //While we don't hit a new line and still in file
            stringStopPtr++;        //Move on to next character in file.
        }
        while((file[stringStartPtr] == ' ' || file[stringStartPtr] == '\t') && stringStartPtr < stringStopPtr)      //Skip leading spaces for indentation;
            stringStartPtr++;
        if(file[stringStartPtr] == '#' || file[stringStartPtr] == '$'){       //This is a comment or empty line

            stringStartPtr = ++stringStopPtr;     //Prepare to move on to next line.
            continue;
        }

        CRdetected = file[stringStopPtr - 1] == '\r' ? 1 : 0;    //Dos formatted line?
        //Copy line in compareBuf.
        strncpy(compareBuf, &file[stringStartPtr], stringStopPtr - CRdetected - stringStartPtr);
        //Manually append terminating character at the end of the string
        compareBuf[stringStopPtr - CRdetected - stringStartPtr] = '\0';
        //if(compareBuf[0] != '\0')
        //printk("\n       %s", compareBuf); //debug, print the whole file line by line.

        stringStartPtr = ++stringStopPtr;     //Prepare to move on to next line.


        //Parse arguments of current line. Up to a max of 5.
        argumentList[0].exist = false;
        argumentList[1].exist = false;
        argumentList[2].exist = false;
        argumentList[3].exist = false;
        argumentList[4].exist = false;
        tempPtr = 0;
        nbArguments = 0;

        for(i = 0; i < 5; i++){
            if(compareBuf[tempPtr] != '\0' && tempPtr < 100){
                argStartPtr[i] = tempPtr;
                tempPtr +=1;
                while(checkEndOfArgument(compareBuf, tempPtr)){       //argument parse
                    tempPtr +=1;
                }
                //Once out, were right after the argument.
                argEndPtr[i] = tempPtr - 1;
                argumentList[i].exist = true;
                nbArguments += 1;
                //Move to start of next argument. Skip '(' too.
                while((compareBuf[tempPtr] == ' ' || compareBuf[tempPtr] == '(') && compareBuf[tempPtr] != '\0'){
                    tempPtr +=1;
                }
            }
            else{
                break;
            }
        }
        //printf("\n\"%s\"", compareBuf);
        //printf("     Line has %u argument(s)", nbArguments);

        //We parsed all 5 possible arguments of a line.
        //Argument 0 contains either a functionCall, a variable declaration or variable itself.
        //We'll start by checking if it's a functionCall.
        for(i = 0; i < 5; i++){
            if(argumentList[i].exist){
                strncpy(tempBuf, &compareBuf[argStartPtr[i]], argEndPtr[i] + 1 - argStartPtr[i]);
                tempBuf[argEndPtr[i] + 1 - argStartPtr[i]] = '\0';
                argumentList[i].text = NULL;
                argumentList[i].type = decodeArgument(tempBuf, &argumentList[i].value, &argumentList[i].text, &variableList);
            }
            else
                break;
        }

        //Reset for new line execution
        arithOpInLine = -1;
        accumulatorInUse = false;

        for(i = 4; i >= 0; i--){
            if(argumentList[i].exist){
                if(argumentList[i].type == ARGTYPE_FUNCTION){       //positive match with a functionCall
                    switch(argumentList[i].value){
                    case FUNCTION_IF:
                            if(!argumentList[i + 1].exist || !argumentList[i + 2].exist || !argumentList[i + 3].exist){
                                    printf("\nRuntime execution error. Missing argument in IF statement!");
                                    goto endExecution;
                            }
                            ifEntry = ifStatementList.first;
                            for(j = 0; j < ifStatementList.count; j++){
                                if(ifEntry->ifPosition == stringStartPtr){
                                    break;
                                }
                                else{
                                    if(ifEntry->next != NULL)
                                        ifEntry = ifEntry->next;
                                }
                            }
                            insideIfStatement = ifFunction(argumentList[i + 1].value, argumentList[i + 2].value, argumentList[i + 3].value);    //Will be 1 if IF condition is valid.
                            if(!insideIfStatement){ //Condition in IF statement was not met
                                if(ifEntry->elsePosition != -1){    //There is a ELSE associated with this IF
                                    stringStartPtr = ifEntry->elsePosition; //Go there

                                }
                                else                                //If there's not ELSE associated with IF
                                    stringStartPtr = ifEntry->endifPosition;        //Go to ENDIF
                                    stringStopPtr = stringStartPtr;
                            }
                            break;
                    case FUNCTION_ELSE:
                            ifEntry = ifStatementList.first;
                            for(j = 0; j < ifStatementList.count; j++){
                                if(ifEntry->elsePosition == stringStartPtr){
                                    break;
                                }
                                else{
                                    if(ifEntry->next != NULL)
                                        ifEntry = ifEntry->next;
                                }
                            }
                            if(insideIfStatement){
                                stringStartPtr = ifEntry->endifPosition;        //Go to ENDIF
                                stringStopPtr = stringStartPtr;
                            }
                            insideIfStatement = 0;
                            break;
                    case FUNCTION_ENDIF:
                            insideIfStatement = 0;
                            break;
                    case FUNCTION_GOTO:
                        if(i == 0 && argumentList[1].exist){
                            labelEntry = labelList.first;
                            for(j = 0; j < labelList.count; j++){
                                if(!strcmp(argumentList[1].text, labelEntry->name)){
                                    stringStartPtr = labelEntry->stringPos;
                                    stringStopPtr = stringStartPtr;
                                    break;
                                }
                                else{
                                    if(labelEntry->next != NULL){
                                        labelEntry = labelEntry->next;
                                    }
                                }
                            }
                        }
                        break;
                    case FUNCTION_GPI:
                        if(argumentList[0].type == ARGTYPE_VARIABLE && argumentList[3].exist){
                            arithAccumulator = gpiFunction((u8)argumentList[3].value);
                            accumulatorInUse = true;
                        }
                        else{
                            printf("\nRuntime execution error. Improper GPI function call!");
                        }
                        break;
                    case FUNCTION_GPO:
                        if(argumentList[1].exist && argumentList[2].exist){
                            gpoFunction((u8)argumentList[1].value, (u8)argumentList[2].value);
                        }
                        else{
                            printf("\nRuntime execution error. Improper GPO function call!");
                        }
                        break;
                    case FUNCTION_WAIT:
                        if(argumentList[1].exist){
                            waitFunction(argumentList[1].value);
                        }
                        else{
                            printf("\nRuntime execution error. Improper WAIT function call!");
                        }
                        break;
                    case FUNCTION_BOOT:
                        if(argumentList[1].exist){
                            bootFunction((u8)argumentList[1].value);
                        }
                        else{
                            printf("\nRuntime execution error. Improper BOOT function call!");
                        }
                        break;
                    case FUNCTION_FAN:
                        if(argumentList[1].exist){
                            fanFunction((u8)argumentList[1].value);
                        }
                        else{
                            printf("\nRuntime execution error. Improper FAN function call!");
                        }
                        break;
                    case FUNCTION_LED:

                        break;
                    case FUNCTION_LCDW:
                        if(argumentList[1].exist && argumentList[2].exist && argumentList[2].text != NULL){
                            lcdPrintFunction((u8)argumentList[1].value, argumentList[2].text);
                        }
                        else{
                            printf("\nRuntime execution error. Improper LCDW function call!");
                        }
                        break;
                    case FUNCTION_LCDC:
                        if(argumentList[1].exist){
                            lcdClearLineFunction((u8)argumentList[1].value);
                        }
                        else{
                            printf("\nRuntime execution error. Improper LCDC function call!");
                        }
                        break;
                    case FUNCTION_LCDR:
                        lcdResetFunction();
                        break;
                    case FUNCTION_LCDB:
                        if(argumentList[1].exist){
                            lcdBacklightFunction((u8)argumentList[1].value);
                        }
                        else{
                            printf("\nRuntime execution error. Improper LCDB function call!");
                        }
                        break;
                    case FUNCTION_LCDP:
                        if(argumentList[1].exist){
                            lcdPowerFunction((u8)argumentList[1].value);
                        }
                        else{
                            printf("\nRuntime execution error. Improper LCDP function call!");
                        }
                        break;
                    case FUNCTION_VAR:
                        if(i == 0 && argumentList[1].exist){
                            if(argumentList[3].exist){
                                variableFunction(argumentList[1].text, argumentList[3].value, &variableList);
                                //printf("\nNew variable: %s = %u", argumentList[1].text, argumentList[3].value);
                            }
                            else{
                                variableFunction(argumentList[1].text, 0, &variableList);
                                //printf("\nNew variable: %s, no init value", argumentList[1].text);
                            }
                        }
                        else{
                            //printf("\nRuntime execution error. Improper variable declaration!");
                            goto endExecution;
                        }
                        break;
                    case FUNCTION_END:
                        printf("\nEND function. Ending script execution gracefully.");
                        goto endExecution;
                        break;
                    default:
                        printf("\nUnknown function call. Halting!");
                        goto endExecution;
                        break;
                    }

                }
                else{                       //Is not a function call
                    if(argumentList[i].type == ARGTYPE_VARIABLE){
                        if(i == 0){ //Assignation to variable
                            if(accumulatorInUse && argumentList[1].type == ARGTYPE_OPERATOR && argumentList[1].value == OPTYPE_EQ){   //There's something to put in it.
                                argumentList[0].value = arithAccumulator;
                                //Update variable content here.
                                if(!updateVariable(argumentList[0].text, argumentList[0].value, &variableList)){
                                    printf("\nRuntime execution error. Undeclared variable");
                                    goto endExecution;
                                }
                                printf("\n%s = %u", argumentList[0].text, argumentList[0].value);
                            }
                        }
                        else{   //Not variable assignation.
                            if(accumulatorInUse){
                                switch(arithOpInLine){
                                    case OPTYPE_PLUS:
                                        arithAccumulator += argumentList[i].value;
                                        break;
                                    case OPTYPE_MINUS:
                                        arithAccumulator = argumentList[i].value - arithAccumulator;
                                        break;
                                    case OPTYPE_MULT:
                                        arithAccumulator *= argumentList[i].value;
                                        break;
                                    case OPTYPE_DIVIDE:
                                        arithAccumulator = argumentList[i].value / arithAccumulator;
                                        break;
                                    case OPTYPE_MOD:
                                        arithAccumulator = argumentList[i].value % arithAccumulator;
                                        break;
                                    default://Accumulator was already used but no arithmetic operator was specified
                                            //printf("\nRuntime execution error. Error in arithmetic operation syntax.");
                                            //goto endExecution;
                                        break;
                                }
                            }
                            else{
                                arithAccumulator = argumentList[i].value;
                                accumulatorInUse = true;
                            }
                        }
                    }
                    else if(argumentList[i].type == ARGTYPE_OPERATOR){
                        arithOpInLine = argumentList[i].value;
                    }
                    else if(argumentList[i].type == ARGTYPE_NUMERIC){
                        if(accumulatorInUse){
                            switch(arithOpInLine){
                                case OPTYPE_PLUS:
                                    arithAccumulator += argumentList[i].value;
                                    break;
                                case OPTYPE_MINUS:
                                    arithAccumulator -= argumentList[i].value;
                                    break;
                                case OPTYPE_MULT:
                                    arithAccumulator *= argumentList[i].value;
                                    break;
                                case OPTYPE_DIVIDE:
                                    arithAccumulator /= argumentList[i].value;
                                    break;
                                case OPTYPE_MOD:
                                    arithAccumulator = argumentList[i].value % arithAccumulator;
                                    break;
                                default://Accumulator was already used but no arithmetic operator was specified
                                        //printf("\nRuntime execution error. Error in arithmetic operation syntax.");
                                        //goto endExecution;
                                    break;
                            }
                        }
                        else{
                            arithAccumulator = argumentList[i].value;
                            accumulatorInUse = true;
                        }
                }
            }
        }
        }
    } //while(stringStopPtr < fileSize)


endExecution:
    //Flush Variable list
    for(i = 0; i < variableList.count; i++){
        if(variableList.first == NULL)
            break;
        currentEntry = variableList.first;
        variableList.first = currentEntry->next;
        free(currentEntry->name);
        free(currentEntry);
    }

    //Flush Label list
    for(i = 0; i < labelList.count; i++){
        if(labelList.first == NULL)
            break;
        labelEntry = labelList.first;
        labelList.first = labelEntry->next;
        free(labelEntry->name);
        free(labelEntry);
    }

    //Flush ifStatement list
    for(i = 0; i < ifStatementList.count; i++){
        if(ifStatementList.first == NULL)
            break;
        ifEntry = ifStatementList.first;
        ifStatementList.first = ifEntry->next;
        free(ifEntry);
    }
    return;
}

bool ifFunction(int param1, u8 op, int param2){
    switch(op){
        case OPTYPE_EQ_EQ: //==
            return (param1 == param2);
            break;
        case OPTYPE_NOT_EQ: //!=
            return (param1 != param2);
            break;
        case OPTYPE_GREATER: //>
            return (param1 > param2);
            break;
        case OPTYPE_LOWER: //<
            return (param1 < param2);
            break;
        case OPTYPE_GREATER_EQ: //>=
            return (param1 >= param2);
            break;
        case OPTYPE_LOWER_EQ: //<=
            return (param1 <= param2);
            break;
        default:
            break;
    }
    return false;
}
bool gpiFunction(u8 port){
    printf("\n****GPI function called, return 1");
    return true;
}
bool gpoFunction(u8 port, u8 value){
    printf("\n****Set GPO port 0x%X with value 0x%X", port, value);
    return true;
}
bool waitFunction(int ms){
    printf("\n****wait_ms for %u seconds", ms);
    return true;
}
bool bootFunction(u8 bank){
    printf("\n****Boot bank: %u", bank);
    return true;
}
bool fanFunction(u8 value){
    printf("\n****Fan speed: %u", value);
    return true;
}
bool ledFunction(char * value){
    printf("\n****LED pattern: %s", value);
    return true;
}
bool lcdPrintFunction(u8 line, char * text){
    printf("\n****LCD Print at line %u : %s", line, text);
    return true;
}
bool lcdClearLineFunction(u8 line){
    printf("\n****LCD clear line %u", line);
    return true;
}
bool lcdResetFunction(void){
    printf("\n****LCD reset screen");
    return true;
}
bool lcdBacklightFunction(u8 value){
    printf("\n****LCD backlight value: %u", value);
    return true;
}
bool lcdPowerFunction(u8 value){
    printf("\n****LCD power %s", value ? "ON": "OFF");
    return true;
}

bool variableFunction(char * name, int initValue, _variableList * variableList){
    variableEntry * newEntry;
    int i;
    char * tempName = (char *)name;


    if(variableList->first == NULL){ //No entry in list yet
        newEntry = (variableEntry *)malloc(sizeof(variableEntry));
        variableList->first = newEntry;
        newEntry->previous = NULL;
    }
    else{
        //Check that requested variable declaration's name is not already used.
        newEntry = variableList->first;
        for(i = 0; i < variableList->count; i++){
            if(!strcmp(newEntry->name, tempName)){
                return false;
            }
            if(newEntry->next != NULL)
                newEntry = newEntry->next;
        }
        newEntry = (variableEntry *)malloc(sizeof(variableEntry));
        variableList->last->next = newEntry;        //Last entry already in list gets to point to this new entry.
        newEntry->previous = variableList->last;    //New entry is now the last one.
    }
    newEntry->next = NULL;
    newEntry->value = initValue;
    newEntry->name = (char *)malloc(strlen(tempName) + 1);
    if(newEntry->name == NULL)
        return false;
    sprintf(newEntry->name, "%s", tempName);

    variableList->last = newEntry;
    variableList->count += 1;

    return true;
}

void parseFileForLabels(u8 * file, u32 fileSize, _labelList * labelList){
    int stringStartPtr = 0, stringStopPtr = 0;
    bool CRdetected;
    char compareBuf[100];
    while(stringStopPtr < fileSize){      //We stay in file
        while(file[stringStopPtr] != '\n' && stringStopPtr < fileSize){        //While we don't hit a new line and still in file
            stringStopPtr++;        //Move on to next character in file.
        }
        while((file[stringStartPtr] == ' ' || file[stringStartPtr] == '\t') && stringStartPtr < stringStopPtr)      //Skip leading spaces for indentation;
            stringStartPtr++;
        if(file[stringStartPtr] == '#'){       //This is a comment or empty line
            stringStartPtr = ++stringStopPtr;     //Prepare to move on to next line.
            continue;
        }
        if(file[stringStartPtr] == '$'){        //Label identifier detected
            //stringStartPtr is now a beginning of the line and stringStopPtr is at the end of it.
            CRdetected = file[stringStopPtr - 1] == '\r' ? 1 : 0;    //Dos formatted line?
            //Copy line in compareBuf.
            strncpy(compareBuf, &file[stringStartPtr], stringStopPtr - CRdetected - stringStartPtr);
            //Manually append terminating character at the end of the string
            compareBuf[stringStopPtr - CRdetected - stringStartPtr] = '\0';
            //if(compareBuf[0] != '\0')
            //printk("\n       %s", compareBuf); //debug, print the whole file line by line.

            if(compareBuf[0] == '$'){             //Label detected.
                addNewLabel(labelList, compareBuf, stringStartPtr);
            }
        }
        stringStartPtr = ++stringStopPtr;     //Prepare to move on to next line.
    }
}

bool parseFileForIFStatements(u8 * file, u32 fileSize, _ifStatementList * ifStatementList){
    int stringStartPtr = 0, stringStopPtr = 0, stringTempPtr;
    bool CRdetected;
    char compareBuf[6];
    ifStatementEntry *newEntry;
    while(stringStopPtr < fileSize){      //We stay in file
        while(file[stringStopPtr] != '\n' && stringStopPtr < fileSize){        //While we don't hit a new line and still in file
            stringStopPtr++;        //Move on to next character in file.
        }
        while((file[stringStartPtr] == ' ' || file[stringStartPtr] == '\t') && stringStartPtr < stringStopPtr)      //Skip leading spaces for indentation;
            stringStartPtr++;
        if(file[stringStartPtr] == '#'){       //This is a comment or empty line
            stringStartPtr = ++stringStopPtr;     //Prepare to move on to next line.
            continue;
        }
        stringTempPtr = stringStartPtr;
        while(file[stringTempPtr] != ' ' && file[stringTempPtr] != '(' && file[stringTempPtr] != '\r' && file[stringTempPtr] != '\n' && file[stringTempPtr] != '\0' && stringTempPtr < (stringStartPtr + 6))
            stringTempPtr += 1;
        if(stringTempPtr >= 2){        //Detected something on this line.
            //stringStartPtr is now a beginning of the line and stringStopPtr is at the end of it.
            //Copy necessary text in compareBuf.
            strncpy(compareBuf, &file[stringStartPtr], stringTempPtr - stringStartPtr);
            //Manually append terminating character at the end of the string
            compareBuf[stringTempPtr - stringStartPtr] = '\0';
            stringStartPtr = ++stringStopPtr;     //Prepare to move on to next line.

            //Search for either IF ELSE or ENDIF strings.
            if(!strcmp(compareBuf, "IF")){
                newEntry = (ifStatementEntry *)malloc(sizeof(ifStatementEntry));
                newEntry->ifPosition = stringStopPtr;    //Cursor position of line below IF statement
                newEntry->elsePosition = -1;             //Not found yet
                newEntry->endifPosition = -1;            //Not found yet
                newEntry->next = NULL;                   //It's the last entry
                newEntry->previous = ifStatementList->last;     //Fill be set to NULL for first entry anyway.
                if(ifStatementList->first == NULL){             //This is the first entry in the list
                    ifStatementList->first = newEntry;
                }
                else{
                    ifStatementList->last->next = newEntry;
                }
                ifStatementList->count += 1;
                ifStatementList->last = newEntry;
            }
            else if(!strcmp(compareBuf, "ELSE")){
                if(ifStatementList->last == NULL){    //Lonesome ELSE statement that isn't linked to any IF statement
                    return false;    //error.
                }
                newEntry = ifStatementList->last;
                while(newEntry->previous != NULL && newEntry->elsePosition != -1){
                    newEntry = newEntry->previous;
                }
                if(newEntry->elsePosition == -1)    //Last sanity check
                    newEntry->elsePosition = stringStopPtr;
                else
                    return false;
            }
            else if(!strcmp(compareBuf, "ENDIF")){
                if(ifStatementList->last == NULL){    //Lonesome ELSE statement that isn't linked to any IF statement
                    return false;    //error.
                }
                newEntry = ifStatementList->last;
                while(newEntry->previous != NULL && newEntry->endifPosition != -1){
                    newEntry = newEntry->previous;
                }
                if(newEntry->endifPosition == -1)    //Last sanity check
                    newEntry->endifPosition = stringStopPtr;
                else
                    return false;
            }
        }
        else
            stringStartPtr = ++stringStopPtr;     //Prepare to move on to next line.
    }
    return true;
}

void addNewLabel(struct _labelList * labelList, char * compareBuf, int position){
    int tempPtr = 1;        //Start checking after '$' character
    labelEntry * newLabelEntry;
    while(compareBuf[tempPtr] != ' ' && compareBuf[tempPtr] != '\0' && tempPtr < 100){
        tempPtr += 1;
    }
    if(tempPtr >= 2){   //There was at least one character after the '$' so we have a label.
        newLabelEntry = (labelEntry *)malloc(sizeof(labelEntry));
        if(labelList->count == 0){       //First entry in list
            labelList->first = newLabelEntry;
            newLabelEntry->previous = NULL;
        }
        else{
            labelList->last->next = newLabelEntry;        //Last entry already in list gets to point to this new entry.
            newLabelEntry->previous = labelList->last;    //New entry is now the last one.
        }
        newLabelEntry->next = NULL;
        newLabelEntry->stringPos = position;      //Place position of next line.
        newLabelEntry->name = (char *)malloc(tempPtr);  //is +1 because of terminating character.
        strncpy(newLabelEntry->name, &compareBuf[1], tempPtr - 1);
        newLabelEntry->name[tempPtr - 1] = '\0';

        labelList->last = newLabelEntry;
        labelList->count += 1;
        printf("\nNew label found in script.\n  Name : %s\n  Points to : %u", newLabelEntry->name, newLabelEntry->stringPos);
    }
}

bool checkEndOfArgument(char * compareBuf, int position){

    if(position >= 1){
        if(compareBuf[position - 1] == '\"' && !stringDeclaration){
            stringDeclaration = true;
            return true;
        }

        if((compareBuf[position - 1] == '\"' || compareBuf[position] == '\0') && stringDeclaration){
            stringDeclaration = false;
            return false;
        }

        //Still in text string
        if(stringDeclaration)
            return true;
    }
    stringDeclaration = false;

    //Start by easy cases where it's clear we're at the end of an argument
    if(compareBuf[position] == ' ')
        return false;
    if(compareBuf[position] == '\0')
        return false;
    if(compareBuf[position] == '(')
        return false;
    if(compareBuf[position] == '#')
        return false;

    //Cases where there is not space between operator and preceding argument
    if(position >= 1 && compareBuf[position - 1] != ' '){
        if(compareBuf[position] == '+' || compareBuf[position] == '-'  || compareBuf[position] == '*'  || compareBuf[position] == '/')
            return false;

        if(compareBuf[position] == '!' || compareBuf[position] == '>'  || compareBuf[position] == '<')
            return false;
    }


    if(position >= 1 && compareBuf[position] != '='){
        if(compareBuf[position - 1] == '>'  || compareBuf[position - 1] == '<')
            return false;
    }

    //Cases where there is not space between operator and following argument
    if(position >= 1 && compareBuf[position] != ' '){
        if(compareBuf[position - 1] == '+' || compareBuf[position - 1] == '-'  || compareBuf[position - 1] == '*'  || compareBuf[position - 1] == '/')
            return false;

        if(compareBuf[position - 1] == '=' && compareBuf[position] != '=')
            return false;
    }



    return true;
}

//Function return indicates what type of argument this is.
//-1 is unknown/skip
//0 is for function call
//1 is for operator
//2 is for variable call
//3 is for numerical value
int decodeArgument(char * inputArg, int * outNum, char ** string, _variableList * variableList){
    int i;
    bool operatorDetected = false;
    variableEntry *tempVariableEntry;

    for(i = 0; i < NBFUNCTIONCALLS; i++){
        if(!strcmp(inputArg, functionCallList[i])){
            //Found a matching function
            *outNum = i;
            *string = functionCallList[i];
            return ARGTYPE_FUNCTION;
        }
    }

    //Check for operators
    if(!strcmp(inputArg, "==" )){
        *outNum = OPTYPE_EQ_EQ;
        operatorDetected = true;
    }
    if(!strcmp(inputArg, "!=")){
        *outNum = OPTYPE_NOT_EQ;
        operatorDetected = true;
    }
    if(!strcmp(inputArg, ">")){
        *outNum = OPTYPE_GREATER;
        operatorDetected = true;
    }
    if(!strcmp(inputArg, "<")){
        *outNum = OPTYPE_LOWER;
        operatorDetected = true;
    }
    if(!strcmp(inputArg, ">=")){
        *outNum = OPTYPE_GREATER_EQ;
        operatorDetected = true;
    }
    if(!strcmp(inputArg, "<=")){
        *outNum = OPTYPE_LOWER_EQ;
        operatorDetected = true;
    }
    if(!strcmp(inputArg, "=")){
        *outNum = OPTYPE_EQ;
        operatorDetected = true;
    }
    if(!strcmp(inputArg, "+")){
        *outNum = OPTYPE_PLUS;
        operatorDetected = true;
    }
    if(!strcmp(inputArg, "-")){
        *outNum = OPTYPE_MINUS;
        operatorDetected = true;
    }
    if(!strcmp(inputArg, "*")){
        *outNum = OPTYPE_MULT;
        operatorDetected = true;
    }
    if(!strcmp(inputArg, "/")){
        *outNum = OPTYPE_DIVIDE;
        operatorDetected = true;
    }
    if(!strcmp(inputArg, "%")){
        *outNum = OPTYPE_MOD;
        operatorDetected = true;
    }

    if(operatorDetected)
        return ARGTYPE_OPERATOR;


    if(inputArg[0] != '\"'){
        tempVariableEntry = variableList->first;
        for(i = 0; i < variableList->count; i++){
            if(!strcmp(tempVariableEntry->name, inputArg)){
                *outNum = tempVariableEntry->value;
                *string = tempVariableEntry->name;
                return ARGTYPE_VARIABLE;
            }
            if(tempVariableEntry->next != NULL)
                tempVariableEntry = tempVariableEntry->next;
            else
                break;
        }

        //Negative number?
        if(inputArg[0] == '-')
            i = 1;
        else
            i = 0;

        if(inputArg[i] >= '0' && inputArg[i] <= '9'){
            *outNum = atoi(&inputArg[i]);
            return ARGTYPE_NUMERIC;
        }
        i = 0;
    }
    else{       //Arg starts with "\""
        i = 2;  //Skip both leading and ending "\""
    }

    //Must be variable call.
    i = strlen(inputArg) - i;
    *string = (char *)malloc(i + 1);
    strncpy(*string, inputArg[0] == '\"' ? &inputArg[1] : inputArg, i);
    *(*string + i) = '\0';


    return ARGTYPE_UNKNOWN;
}

bool updateVariable(char * name, int value, _variableList * variableList){
    variableEntry * currentEntry = variableList->first;
    int i;

    for(i = 0; i < variableList->count; i++){
        if(!strcmp(currentEntry->name, name)){
            currentEntry->value = value;
            break;
        }
        if(currentEntry->next != NULL)
            currentEntry = currentEntry->next;
        else{
            return false;
        }
    }
    return true;
}
