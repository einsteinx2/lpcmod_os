/*
 * scriptChecker.c
 *
 *  Created on: Jan 16, 2015
 *      Author: bennyboy
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef unsigned char u8;
typedef unsigned int u32;
typedef unsigned short u16;

void runScript(u8 * file, u32 fileSize, void * param);

bool ifFunction(void * param1, void * op, void * param2);
bool gpiFunction(void * port, void * ignored1, void * ignored2);
bool gpoFunction(void * port, void * value, void * ignored);
bool waitFunction(void * ms, void * ignored1, void * ignored2);
bool bootFunction(void * bank, void * ignored1, void * ignored2);
bool endFunction(void * ignored, void * ignored1, void * ignored2);
bool fanFunction(void * value, void * ignored1, void * ignored2);
bool ledFunction(void * value, void * ignored1, void * ignored2);
bool lcdPrintFunction(void * line, void * text, void * ignored);
bool lcdClearLineFunction(void * line, void * ignored1, void * ignored2);
bool lcdResetFunction(void * ignored, void * ignored1, void * ignored2);
bool lcdBacklightFunction(void * value, void * ignored1, void * ignored2);
bool lcdPowerFunction(void * value, void * ignored1, void * ignored2);
bool variableFunction(void * name, void * initValue, void * variableList);
bool checkEndOfArgument(char * compareBuf, int position);



#define NBFUNCTIONCALLS 18

typedef struct {
    char* functionName;
    bool (*functionPtr) (void *, void *, void *);
}functionCall;

functionCall functionCallList[NBFUNCTIONCALLS] = {
        { "IF", &ifFunction},
        { "ELSE", NULL},
        { "ENDIF", NULL},
        { "GOTO", NULL},
        { "GPI", &gpiFunction},
        { "GPO", &gpoFunction},
        { "WAIT", &waitFunction},
        { "BOOT", &bootFunction},
        { "END", &endFunction},
        { "FAN", &fanFunction},
        { "LED", &ledFunction},
        { "LCDP", &lcdPrintFunction},
        { "LCDC", &lcdClearLineFunction},
        { "LCDR", &lcdResetFunction},
        { "LCDB", &lcdBacklightFunction},
        { "LCDP", &lcdPowerFunction},
        { "VAR", &variableFunction},
        { "END", NULL}
};

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

void parseFileForLabels(u8 * file, u32 fileSize, _labelList * labelList);
void addNewLabel(struct _labelList * labelList, char * compareBuf, int position);
int decodeArgument(char * inputArg, int * outNum, _variableList * variableList);

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
    runScript(buffer, fileSize, NULL);

    free(buffer);

	return EXIT_SUCCESS;
}


void showUsage(const char *progname) {
    printf("Usage:   ");
    printf("%s 'input file'\n",progname);
}

void runScript(u8 * file, u32 fileSize, void * param){
    bool runExecution = true; //True while not at the end of script or END command isn't read.
    bool variableAssignation;
    int functionCallFlag;        //-1 is no functionCall. Other is representative of functionCall list index.
    bool functionCallResult;
    int i;
    int decodedArg[5][2];
    _labelList labelList;
    labelEntry * labelEntry;

    _variableList variableList;
    variableEntry * currentEntry;



    int stringStartPtr = 0, stringStopPtr = 0, tempPtr, savedPosPtr;
    int argStartPtr[5], argEndPtr[5];
    bool argExist[5];
    int nbArguments;
    bool CRdetected;
    char compareBuf[100];                     //100 character long seems acceptable
    char tempBuf[50];

    variableList.count = 0;
    variableList.first = NULL;
    variableList.last = NULL;
    labelList.count = 0;
    labelList.first = NULL;
    labelList.last = NULL;


    //Parse for labels first
    //Put all found in labelList
    parseFileForLabels(file, fileSize, &labelList);
    printf("\n\nTotal labels found : %u", labelList.count);
    printf("\n\nBegin script execution");

    while(stringStopPtr < fileSize){      //We stay in file
        while(file[stringStopPtr] != '\n' && stringStopPtr < fileSize){        //While we don't hit a new line and still in file
            stringStopPtr++;        //Move on to next character in file.
        }
        while(file[stringStartPtr] == ' ' && stringStartPtr < stringStopPtr)      //Skip leading spaces for indentation;
            stringStartPtr++;
        if(file[stringStartPtr] == '#' || file[stringStartPtr] == '%'){       //This is a comment or empty line

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
        argExist[0] = false;
        argExist[1] = false;
        argExist[2] = false;
        argExist[3] = false;
        argExist[4] = false;
        variableAssignation = false;
        tempPtr = 0;
        nbArguments = 0;

        for(i = 0; i < 5; i++){
            if(compareBuf[tempPtr] != '\0' && tempPtr < 100){
                argStartPtr[i] = tempPtr;
                while(checkEndOfArgument(compareBuf, tempPtr)){       //argument parse
                    tempPtr +=1;
                }
                //Once out, were right after the argument.
                argEndPtr[i] = tempPtr - 1;
                argExist[i] = true;
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
        printf("\n\"%s\"", compareBuf);
        printf("     Line has %u argument(s)", nbArguments);

        //We parsed all 5 possible arguments of a line.
        //Argument 0 contains either a functionCall, a variable declaration or variable itself.
        //We'll start by checking if it's a functionCall.
        for(i = 0; i < 5; i++){
            if(argExist[i]){
                strncpy(tempBuf, compareBuf, argEndPtr[i] + 1 - argStartPtr[i]);
                tempBuf[argEndPtr[0] + 1] = '\0';
                decodedArg[i][0] = decodeArgument(tempBuf, &decodedArg[i][1], &variableList);
            }
            else
                break;
        }


        if(functionCallFlag){       //positive match with a functionCall
            //functionCallResult = (functionCallList[i].functionPtr)
        }
        else{                       //Is variable

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
    return;
}

bool ifFunction(void * param1, void * op, void * param2){
    int * tempParam1 = (int *)param1;
    int * tempParam2 = (int *)param2;
    switch(*(u8 *)op){
        case 0: //==
            return (tempParam1 == tempParam2);
            break;
        case 1: //!=
            return (tempParam1 != tempParam2);
            break;
        case 2: //>
            return (tempParam1 > tempParam2);
            break;
        case 3: //<
            return (tempParam1 < tempParam2);
            break;
        case 4: //>=
            return (tempParam1 >= tempParam2);
            break;
        case 5: //<=
            return (tempParam1 <= tempParam2);
            break;
        default:
            break;
    }
    return false;
}
bool gpiFunction(void * port, void * ignored1, void * ignored2){
    return true;
}
bool gpoFunction(void * port, void * value, void * ignored){
    printf("\n Set GPO port 0x%X with value 0x%X", *(u8 *)port, *(u8 *)value);
    return true;
}
bool waitFunction(void * ms, void * ignored1, void * ignored2){
    printf("\n wait_ms for %u seconds", *(int *)ms);
    return true;
}
bool bootFunction(void * bank, void * ignored1, void * ignored2){
    printf("\n Boot bank: %u", *(u8 *)bank);
    return true;
}
bool endFunction(void * ignored, void * ignored1, void * ignored2){
    printf("\n END function");
    return true;
}
bool fanFunction(void * value, void * ignored1, void * ignored2){
    printf("\n Fan speed: %u", *(u8 *)value);
    return true;
}
bool ledFunction(void * value, void * ignored1, void * ignored2){
    printf("\n LED pattern: %s", (char *)value);
    return true;
}
bool lcdPrintFunction(void * line, void * text, void * ignored){
    printf("\n LCD Print at line %u : %s", *(u8 *)line, (char *)text);
    return true;
}
bool lcdClearLineFunction(void * line, void * ignored1, void * ignored2){
    printf("\n LCD clear line %u", *(u8 *)line);
    return true;
}
bool lcdResetFunction(void * ignored, void * ignored1, void * ignored2){
    printf("\n LCD reset screen");
    return true;
}
bool lcdBacklightFunction(void * value, void * ignored1, void * ignored2){
    printf("\n LCD backlight value: %u", *(u8 *)value);
    return true;
}
bool lcdPowerFunction(void * value, void * ignored1, void * ignored2){
    printf("\n LCD power %s", *(u8 *)value ? "ON": "OFF");
    return true;
}

bool variableFunction(void * name, void * initValue, void * variableList){
    _variableList * tempVariableList = (_variableList *)variableList;
    variableEntry * newEntry;
    int i;
    char * tempName = (char *)name;
    newEntry = (variableEntry *)malloc(sizeof(variableEntry));

    if(tempVariableList->first == NULL){ //No entry in list yet
        tempVariableList->first = newEntry;
        newEntry->previous = NULL;
    }
    else{
        //Check that requested variable declaration's name is not already used.
        newEntry = tempVariableList->first;
        for(i = 0; i < tempVariableList->count; i++){
            if(!strcmp(newEntry->name, tempName)){
                return false;
            }
            if(newEntry->next != NULL)
                newEntry = newEntry->next;
        }
        tempVariableList->last->next = newEntry;        //Last entry already in list gets to point to this new entry.
        newEntry->previous = tempVariableList->last;    //New entry is now the last one.
    }
    newEntry->next = NULL;
    newEntry->value = *(int *)initValue;
    newEntry->name = (char *)malloc(strlen(tempName) + 1);
    if(newEntry->name == NULL)
        return false;
    sprintf(newEntry->name, "%s", tempName);

    tempVariableList->last = newEntry;
    tempVariableList->count += 1;

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
        while(file[stringStartPtr] == ' ' && stringStartPtr < stringStopPtr)      //Skip leading spaces for indentation;
            stringStartPtr++;
        if(file[stringStartPtr] == '#'){       //This is a comment or empty line
            stringStartPtr = ++stringStopPtr;     //Prepare to move on to next line.
            continue;
        }
        if(file[stringStartPtr] == '%'){        //Label identifier detected
            //stringStartPtr is now a beginning of the line and stringStopPtr is at the end of it.
            CRdetected = file[stringStopPtr - 1] == '\r' ? 1 : 0;    //Dos formatted line?
            //Copy line in compareBuf.
            strncpy(compareBuf, &file[stringStartPtr], stringStopPtr - CRdetected - stringStartPtr);
            //Manually append terminating character at the end of the string
            compareBuf[stringStopPtr - CRdetected - stringStartPtr] = '\0';
            //if(compareBuf[0] != '\0')
            //printk("\n       %s", compareBuf); //debug, print the whole file line by line.

            if(compareBuf[0] == '%'){             //Label detected.
                addNewLabel(labelList, compareBuf, stringStartPtr);
            }
        }
        stringStartPtr = ++stringStopPtr;     //Prepare to move on to next line.
    }
}

void addNewLabel(struct _labelList * labelList, char * compareBuf, int position){
    int tempPtr = 1;        //Start checking after '%' character
    labelEntry * newLabelEntry;
    while(compareBuf[tempPtr] != ' ' && compareBuf[tempPtr] != '\0' && tempPtr < 100){
        tempPtr += 1;
    }
    if(tempPtr >= 2){   //There was at least one character after the '%' so we have a label.
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

    //Cases where there is not space between operator and following argument
    if(position >= 1 && compareBuf[position] != ' '){
        if(compareBuf[position - 1] == '+' || compareBuf[position - 1] == '-'  || compareBuf[position - 1] == '*'  || compareBuf[position - 1] == '/')
            return false;

        if(compareBuf[position - 1] == '!' || compareBuf[position - 1] == '>'  || compareBuf[position - 1] == '<')
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
int decodeArgument(char * inputArg, int * outNum, _variableList * variableList){
    int i;
    variableEntry *tempVariableEntry;

    for(i = 0; i < NBFUNCTIONCALLS; i++){
        if(!strcmp(inputArg, functionCallList[i].functionName)){
            //Found a matching function
            *outNum = i;
            return 0;
        }
    }

    //Check for operators
    if(!strcmp(inputArg, "==")){
        *outNum = 0;
        return 1;
    }
    if(!strcmp(inputArg, "!=")){
        *outNum = 1;
        return 1;
    }
    if(!strcmp(inputArg, ">")){
        *outNum = 2;
        return 1;
    }
    if(!strcmp(inputArg, "<")){
        *outNum = 3;
        return 1;
    }
    if(!strcmp(inputArg, ">=")){
        *outNum = 4;
        return 1;
    }
    if(!strcmp(inputArg, "<=")){
        *outNum = 5;
        return 1;
    }
    if(!strcmp(inputArg, "=")){
        *outNum = 6;
        return 1;
    }
    if(!strcmp(inputArg, "+")){
        *outNum = 7;
        return 1;
    }
    if(!strcmp(inputArg, "-")){
        *outNum = 8;
        return 1;
    }
    if(!strcmp(inputArg, "*")){
        *outNum = 9;
        return 1;
    }
    if(!strcmp(inputArg, "/")){
        *outNum = 10;
        return 1;
    }

    tempVariableEntry = variableList->first;
    for(i = 0; i < variableList->count; i++){
        if(!strcmp(tempVariableEntry->name, inputArg)){
            *outNum = tempVariableEntry->value;
            return 2;
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

    if(inputArg[i] >= '0' && inputArg[i] >= '9'){
        *outNum = atoi(&inputArg[i]);
        return 3;
    }

    //Nothing good found. Throw error please.
    return -1;
}
