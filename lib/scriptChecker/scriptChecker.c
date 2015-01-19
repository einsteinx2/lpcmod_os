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

#define NBFUNCTIONCALLS 18

typedef struct {
    char* functionName;
    bool (*functionPtr) (void *, void *, void *);
}functionCall;

typedef struct variableEntry{
    struct variableEntry * previous;
    struct variableEntry * next;
    int * value;
    char * name;
}variableEntry;

typedef struct _variableList{
    int count;
    variableEntry * first;
    variableEntry * last;
}_variableList;

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
    _labelList labelList;
    labelEntry * newLabelEntry;

    _variableList variableList;
    variableEntry * currentEntry;

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

    int stringStartPtr = 0, stringStopPtr = 0, valueStartPtr = 0, tempPtr, savedPosPtr;
    int arg1EndPtr, arg2StartPtr, arg2EndPtr, arg3StartPtr, arg3EndPtr, arg4StartPtr, arg4EndPtr, arg5StartPtr, arg5EndPtr;
    bool argExist[5];
    bool CRdetected;
    char compareBuf[100];                     //100 character long seems acceptable
    char tempBuf[50];

    variableList.count = 0;
    variableList.first = NULL;
    variableList.last = NULL;
    labelList.count = 0;
    labelList.first = NULL;
    labelList.last = NULL;

    while(stringStopPtr < fileSize){      //We stay in file
        while(file[stringStopPtr] != '\n' && stringStopPtr < fileSize){        //While we don't hit a new line and still in file
            stringStopPtr++;        //Move on to next character in file.
        }
        while(file[stringStartPtr] == ' ' && stringStartPtr < stringStopPtr)      //Skip leading spaces for indentation;
            stringStartPtr++;
        if(file[stringStartPtr] == '#' ){       //This is a comment or empty line

            stringStartPtr = ++stringStopPtr;     //Prepare to move on to next line.
            continue;
        }

        //stringStartPtr is now a beginning of the line and stringStopPtr is at the end of it.
        valueStartPtr = 0;
        CRdetected = file[stringStopPtr - 1] == '\r' ? 1 : 0;    //Dos formatted line?
        //Copy line in compareBuf.
        strncpy(compareBuf, &file[stringStartPtr], stringStopPtr - CRdetected - stringStartPtr);
        //Manually append terminating character at the end of the string
        compareBuf[stringStopPtr - CRdetected - stringStartPtr] = '\0';
        //if(compareBuf[0] != '\0')
        //printk("\n       %s", compareBuf); //debug, print the whole file line by line.

        stringStartPtr = ++stringStopPtr;     //Prepare to move on to next line.
        if(compareBuf[0] == '%'){             //Label detected.
            tempPtr = 1;        //Start checking after '%' character
            while(compareBuf[tempPtr] != ' ' && compareBuf[tempPtr] != '\0' && tempPtr < 100){
                tempPtr += 1;
            }
            if(tempPtr >= 2){   //There was at least one character after the '%' so we have a label.
                newLabelEntry = (labelEntry *)malloc(sizeof(labelEntry));
                if(labelList.count == 0){       //First entry in list
                    labelList.first = newLabelEntry;
                    newLabelEntry->previous = NULL;
                }
                else{
                    labelList.last->next = newLabelEntry;        //Last entry already in list gets to point to this new entry.
                    newLabelEntry->previous = labelList.last;    //New entry is now the last one.
                }
                newLabelEntry->next = NULL;
                newLabelEntry->stringPos = stringStartPtr;      //Place position of next line.
                newLabelEntry->name = (char *)malloc(tempPtr);  //is +1 because of terminating character.
                strncpy(newLabelEntry->name, &compareBuf[1], tempPtr - 1);
                newLabelEntry->name[tempPtr - 1] = '\0';

                labelList.last = newLabelEntry;
                labelList.count += 1;
            }
        }
        //It's not a label
        else{
            argExist[0] = false;
            argExist[1] = false;
            argExist[2] = false;
            argExist[3] = false;
            argExist[4] = false;
            variableAssignation = false;
            tempPtr = 0;
            //compareBuf shouldn't start with ' ' characters.
            while(compareBuf[tempPtr] != '\0' && tempPtr < 100){
                while(compareBuf[tempPtr] != ' ' && compareBuf[tempPtr] != '\0' && compareBuf[tempPtr] != '('){       //First argument parse
                    tempPtr +=1;
                }
                //Once out, were right after the first argument of the line.
                arg1EndPtr = tempPtr - 1;
                argExist[0] = true;
                //Move to start of second argument. Skip '(' too.
                while((compareBuf[tempPtr] == ' ' || compareBuf[tempPtr] == '(') && compareBuf[tempPtr] != '\0'){
                    tempPtr +=1;
                }
                //Make sure we're not at the end of the line
                if(compareBuf[tempPtr] != '\0'){
                    arg2StartPtr = tempPtr;
                    while(compareBuf[tempPtr] != ' ' && compareBuf[tempPtr] != '\0' && compareBuf[tempPtr] != '('){       //2nd argument parse
                        tempPtr +=1;
                    }
                    //Once out, were right after the 2nd argument of the line.
                    arg2EndPtr = tempPtr - 1;
                    argExist[1] = true;
                    //Move to start of third argument.
                    while((compareBuf[tempPtr] == ' ' || compareBuf[tempPtr] == '(') && compareBuf[tempPtr] != '\0'){
                        tempPtr +=1;
                    }
                    //Make sure we're not at the end of the line
                    if(compareBuf[tempPtr] != '\0'){
                        arg3StartPtr = tempPtr;
                        while(compareBuf[tempPtr] != ' ' && compareBuf[tempPtr] != '\0' && compareBuf[tempPtr] != '('){       //2nd argument parse
                            tempPtr +=1;
                        }
                        //Once out, were right after the 3rd argument of the line.
                        arg3EndPtr = tempPtr - 1;
                        argExist[2] = true;
                        //Move to start of fourth argument.
                        while((compareBuf[tempPtr] == ' ' || compareBuf[tempPtr] == '(') && compareBuf[tempPtr] != '\0'){
                            tempPtr +=1;
                        }
                        //Make sure we're not at the end of the line
                        if(compareBuf[tempPtr] != '\0'){
                            arg4StartPtr = tempPtr;
                            while(compareBuf[tempPtr] != ' ' && compareBuf[tempPtr] != '\0' && compareBuf[tempPtr] != '('){       //2nd argument parse
                                tempPtr +=1;
                            }
                            //Once out, were right after the 4th argument of the line.
                            arg4EndPtr = tempPtr - 1;
                            argExist[3] = true;
                            //Move to start of fifth argument.
                            while((compareBuf[tempPtr] == ' ' || compareBuf[tempPtr] == '(') && compareBuf[tempPtr] != '\0'){
                                tempPtr +=1;
                            }
                            if(compareBuf[tempPtr] != '\0'){
                                arg5StartPtr = tempPtr;
                                while(compareBuf[tempPtr] != ' ' && compareBuf[tempPtr] != '\0' && compareBuf[tempPtr] != '('){       //2nd argument parse
                                    tempPtr +=1;
                                }
                                //Once out, were right after the 4th argument of the line.
                                arg5EndPtr = tempPtr - 1;
                                argExist[4] = true;
                            }
                        }
                    }
                }
            }
            //We parsed all 4 possible arguments of a line.
            //Argument 1 contains either a functionCall, a variable declaration or variable itself.
            //We'll start by checking if it's a functionCall.
            strncpy(tempBuf, compareBuf, arg1EndPtr + 1);
            tempBuf[arg1EndPtr + 1] = '\0';
            functionCallFlag = -1;
            for(i = 0; i < NBFUNCTIONCALLS; i++){
                if(!strcmp(tempBuf, functionCallList[i].functionName)){
                    //Found a matching function
                    functionCallFlag = i;
                    break;
                }
            }

            if(argExist[1]){
                strncpy(tempBuf, &compareBuf[arg2StartPtr], arg2EndPtr - arg2StartPtr + 1);
                tempBuf[arg2EndPtr - arg2StartPtr + 1] = '\0';
                if(tempBuf[0] == '='){
                    variableAssignation = true;
                }
                else{
                    //Is argument of a function call. Variable or literal.

                }
            }


            if(functionCallFlag){       //positive match with a functionCall
                //functionCallResult = (functionCallList[i].functionPtr)
            }
            else{                       //Is variable

            }
        }
    } //while(stringStopPtr < fileSize)


    //Flush Variable list
    for(i = 0; i < variableList.count; i++){
        if(variableList.first == NULL)
            break;
        currentEntry = variableList.first;
        variableList.first = currentEntry->next;
        free(currentEntry->name);
        free(currentEntry->value);
        free(currentEntry);
    }

    //Flush Label list
    for(i = 0; i < labelList.count; i++){
        if(labelList.first == NULL)
            break;
        newLabelEntry = labelList.first;
        labelList.first = newLabelEntry->next;
        free(newLabelEntry->name);
        free(newLabelEntry);
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
    return true;
}
bool waitFunction(void * ms, void * ignored1, void * ignored2){
    return true;
}
bool bootFunction(void * bank, void * ignored1, void * ignored2){
    return true;
}
bool endFunction(void * ignored, void * ignored1, void * ignored2){
    return true;
}
bool fanFunction(void * value, void * ignored1, void * ignored2){
    return true;
}
bool ledFunction(void * value, void * ignored1, void * ignored2){
    return true;
}
bool lcdPrintFunction(void * line, void * text, void * ignored){
    return true;
}
bool lcdClearLineFunction(void * line, void * ignored1, void * ignored2){
    return true;
}
bool lcdResetFunction(void * ignored, void * ignored1, void * ignored2){
    return true;
}
bool lcdBacklightFunction(void * value, void * ignored1, void * ignored2){
    return true;
}
bool lcdPowerFunction(void * value, void * ignored1, void * ignored2){

    return true;
}

bool variableFunction(void * name, void * initValue, void * variableList){
    _variableList * tempVariableList = (_variableList *)variableList;
    variableEntry * newEntry;
    char * tempName = (char *)name;
    newEntry = (variableEntry *)malloc(sizeof(variableEntry));

    if(tempVariableList->first == NULL){ //No entry in list yet
        tempVariableList->first = newEntry;
        newEntry->previous = NULL;
    }
    else{
        tempVariableList->last->next = newEntry;        //Last entry already in list gets to point to this new entry.
        newEntry->previous = tempVariableList->last;    //New entry is now the last one.
    }
    newEntry->next = NULL;
    newEntry->value = (int *)malloc(sizeof(int));
    if(newEntry->value == NULL)
        return false;
    *newEntry->value = *(int *)initValue;
    newEntry->name = (char *)malloc(strlen(tempName) + 1);
    if(newEntry->name == NULL)
        return false;
    sprintf(newEntry->name, "%s", tempName);

    tempVariableList->last = newEntry;
    tempVariableList->count += 1;

    return true;
}
