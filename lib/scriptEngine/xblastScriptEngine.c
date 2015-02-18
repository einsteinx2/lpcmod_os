/*
 * xblastScriptEngine.c
 *
 *  Created on: Jan 14, 2015
 *      Author: bennydiamond
 */

#include "boot.h"
#include "xblastScriptEngine.h"
#include "lpcmod_v1.h"
#include "lib/LPCMod/BootLCD.h"



#define ARGTYPE_UNKNOWN -1
#define ARGTYPE_FUNCTION 0
#define ARGTYPE_OPERATOR 1
#define ARGTYPE_VARIABLE 2
#define ARGTYPE_NUMERIC  3
#define ARGTYPE_STRING   4

#define ARGTYPE_UNKNOWN -1
#define ARGTYPE_FUNCTION 0
#define ARGTYPE_OPERATOR 1
#define ARGTYPE_VARIABLE 2
#define ARGTYPE_NUMERIC  3

#define OPTYPE_EQ_EQ    0
#define OPTYPE_NOT_EQ   1
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
#define OPTYPE_SHIFTUP  12
#define OPTYPE_SHIFTDOWN   13

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
#define FUNCTION_SPIR   16
#define FUNCTION_SPIW   17
#define FUNCTION_XPAD   18
#define FUNCTION_END    19

#define NBFUNCTIONCALLS 20


typedef struct variableEntry{
    struct variableEntry * previous;
    struct variableEntry * next;
    int value;
    char * name;
    bool readOnly;
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
    bool validAssertion;
    struct ifStatementEntry *next;
    struct ifStatementEntry *previous;
}ifStatementEntry;

typedef struct _ifStatementList{
    int count;
    ifStatementEntry *first;
    ifStatementEntry *last;
}_ifStatementList;

bool stringDeclaration;

bool ifFunction(int param1, u8 op, int param2);
bool gpiFunction(u8 port);
bool gpoFunction(u8 port, u8 value);
bool waitFunction(int ms);
bool bootFunction(u8 bank);
bool fanFunction(u8 value);
bool ledFunction(char * value);
bool lcdPrintFunction(u8 line, char * text, u8 stringLength);
bool lcdClearLineFunction(u8 line);
bool lcdResetFunction(void);
bool lcdBacklightFunction(u8 value);
bool lcdPowerFunction(u8 value);
u8 SPIRead(void);
bool SPIWrite(u8 data);
u8 XPADRead(void);
bool variableFunction(char * name, int initValue, _variableList * variableList, bool readOnly);
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
        "SPIR",
        "SPIW",
        "XPAD",
        "END"
};


void parseFileForLabels(u8 * file, u32 fileSize, _labelList * labelList);
void addNewLabel(struct _labelList * labelList, char * compareBuf, int position);
bool parseFileForIFStatements(u8 * file, u32 fileSize, _ifStatementList * ifStatementList);
int decodeArgument(char * inputArg, int * outNum, char ** string, _variableList * variableList);

void runScript(u8 * file, u32 fileSize, int paramCount, int * param){
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
    char compareBuf[100];                     //100 character long seems acceptable
    char tempBuf[50];

    for(i = 0; i < 5; i++){
        argumentList[i].exist = false;
        argumentList[i].text = NULL;
        argumentList[i].type = ARGTYPE_UNKNOWN;
        argumentList[i].value = 0;
    }
    stringDeclaration = false;

    variableList.count = 0;
    variableList.first = NULL;
    variableList.last = NULL;
    //Add parameters passed in runScript function call as variables.
    for(i = 0; i < paramCount; i++){
        sprintf(tempBuf, "_param%u", i);
        variableFunction(tempBuf, param[i], &variableList, false);
    }
    variableFunction("BNK512", BNK512, &variableList, true);
    variableFunction("BNK256", BNK256, &variableList, true);
    variableFunction("BNKOS", BNKOS, &variableList, true);
    variableFunction("BNKFULLTSOP", BNKFULLTSOP, &variableList, true);
    variableFunction("BNKTSOPSPLIT0", BNKTSOPSPLIT0, &variableList, true);
    variableFunction("BNKTSOPSPLIT1", BNKTSOPSPLIT1, &variableList, true);

    variableFunction("xpadA", TRIGGER_XPAD_KEY_A + 1, &variableList, true);
    variableFunction("xpadB", TRIGGER_XPAD_KEY_B + 1, &variableList, true);
    variableFunction("xpadX", TRIGGER_XPAD_KEY_X + 1, &variableList, true);
    variableFunction("xpadY", TRIGGER_XPAD_KEY_Y + 1, &variableList, true);
    variableFunction("xpadBl", TRIGGER_XPAD_KEY_BLACK + 1, &variableList, true);
    variableFunction("xpadW", TRIGGER_XPAD_KEY_WHITE + 1, &variableList, true);


    labelList.count = 0;
    labelList.first = NULL;
    labelList.last = NULL;
    ifStatementList.count = 0;
    ifStatementList.first = NULL;
    ifStatementList.last = NULL;

    //Parse for labels first
    //Put all found in labelList
    parseFileForLabels(file, fileSize, &labelList);
    //printf("\n\nTotal labels found : %u", labelList.count);

    if(!parseFileForIFStatements(file, fileSize, &ifStatementList)){
        //printf("\n\nError in IF/ELSE/ENDIF statement logic!!!!");
        goto endExecution;
    }
    //printf("\n\nTotal IF/ELSE/ENDIF statements found : %u", ifStatementList.count);
    //printf("\n\nBegin script execution");

    while(stringStopPtr < fileSize){      //We stay in file
        while(file[stringStopPtr] != '\n' && stringStopPtr < fileSize){        //While we don't hit a new line and still in file
            stringStopPtr++;        //Move on to next character in file.
        }

        if(file[stringStartPtr] == '$'){       //This is a label line
            stringStartPtr = ++stringStopPtr;     //Prepare to move on to next line.
            continue;
        }

        //Copy line in compareBuf.
        strncpy(compareBuf, &file[stringStartPtr], stringStopPtr - stringStartPtr);
        //Manually append terminating character at the end of the string
        compareBuf[stringStopPtr - stringStartPtr] = '\0';
        //if(compareBuf[0] != '\0')
        //printk("\n       %s", compareBuf); //debug, print the whole file line by line.

        stringStartPtr = ++stringStopPtr;     //Prepare to move on to next line.


        //Parse arguments of current line. Up to a max of 5.
        for(i = 0; i < 5; i++){
            argumentList[i].exist = false;
            if(argumentList[i].type == ARGTYPE_STRING){ //ARGTYPE_STRING indicates there has been a malloc
                free(argumentList[i].text);
                argumentList[i].text = NULL;
            }
        }
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
                while((compareBuf[tempPtr] == '(' || compareBuf[tempPtr] == ',' || compareBuf[tempPtr] == ')') && compareBuf[tempPtr] != '\0'){
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
                                    //printf("\nRuntime execution error. Missing argument in IF statement!");
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
                            ifEntry->validAssertion = ifFunction(argumentList[i + 1].value, argumentList[i + 2].value, argumentList[i + 3].value);    //Will be 1 if IF condition is valid.
                            if(!ifEntry->validAssertion){ //Condition in IF statement was not met
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
                            if(ifEntry->validAssertion){
                                stringStartPtr = ifEntry->endifPosition;        //Go to ENDIF
                                stringStopPtr = stringStartPtr;
                            }
                            break;
                    case FUNCTION_ENDIF:
                            for(j = 0; j < ifStatementList.count; j++){
                                if(ifEntry->endifPosition == stringStartPtr){
                                    break;
                                }
                                else{
                                    if(ifEntry->next != NULL)
                                        ifEntry = ifEntry->next;
                                }
                            }
                            ifEntry->validAssertion = false;
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
                        //else{
                            //printf("\nRuntime execution error. Improper GPI function call!");
                        //}
                        break;
                    case FUNCTION_GPO:
                        if(argumentList[1].exist && argumentList[2].exist){
                            gpoFunction((u8)argumentList[1].value, (u8)argumentList[2].value);
                        }
                        //else{
                            //printf("\nRuntime execution error. Improper GPO function call!");
                        //}
                        break;
                    case FUNCTION_WAIT:
                        if(argumentList[1].exist && (argumentList[1].type == ARGTYPE_VARIABLE || argumentList[1].type == ARGTYPE_NUMERIC)){
                            waitFunction(argumentList[1].value);
                        }
                        //else{
                            //printf("\nRuntime execution error. Improper WAIT function call!");
                        //}
                        break;
                    case FUNCTION_BOOT:
                        if(argumentList[1].exist && (argumentList[1].type == ARGTYPE_VARIABLE || argumentList[1].type == ARGTYPE_NUMERIC)){
                            bootFunction((u8)argumentList[1].value);
                        }
                        //else{
                            //printf("\nRuntime execution error. Improper BOOT function call!");
                        //}
                        break;
                    case FUNCTION_FAN:
                        if(argumentList[1].exist && (argumentList[1].type == ARGTYPE_VARIABLE || argumentList[1].type == ARGTYPE_NUMERIC)){
                            fanFunction((u8)argumentList[1].value);
                        }
                        //else{
                            //printf("\nRuntime execution error. Improper FAN function call!");
                        //}
                        break;
                    case FUNCTION_LED:
                        if(argumentList[1].exist && argumentList[1].text != NULL){
                            ledFunction(argumentList[1].text);
                        }
                        //else{
                            //printf("\nRuntime execution error. Improper LED function call!");
                        //}
                        break;
                    case FUNCTION_LCDW:
                        if(argumentList[1].exist && argumentList[2].exist && argumentList[2].text != NULL){
                            if(argumentList[3].exist && (argumentList[3].type == ARGTYPE_VARIABLE || argumentList[3].type == ARGTYPE_NUMERIC))
                                lcdPrintFunction((u8)argumentList[1].value, argumentList[2].text, argumentList[3].value);
                            else
                                lcdPrintFunction((u8)argumentList[1].value, argumentList[2].text, xLCD.LineSize);
                        }
                        //else{
                            //printf("\nRuntime execution error. Improper LCDW function call!");
                        //}
                        break;
                    case FUNCTION_LCDC:
                        if(argumentList[1].exist && (argumentList[1].type == ARGTYPE_VARIABLE || argumentList[1].type == ARGTYPE_NUMERIC)){
                            lcdClearLineFunction((u8)argumentList[1].value);
                        }
                        //else{
                            //printf("\nRuntime execution error. Improper LCDC function call!");
                        //}
                        break;
                    case FUNCTION_LCDR:
                        lcdResetFunction();
                        break;
                    case FUNCTION_LCDB:
                        if(argumentList[1].exist && (argumentList[1].type == ARGTYPE_VARIABLE || argumentList[1].type == ARGTYPE_NUMERIC)){
                            lcdBacklightFunction((u8)argumentList[1].value);
                        }
                        //else{
                            //printf("\nRuntime execution error. Improper LCDB function call!");
                        //}
                        break;
                    case FUNCTION_LCDP:
                        if(argumentList[1].exist && (argumentList[1].type == ARGTYPE_VARIABLE || argumentList[1].type == ARGTYPE_NUMERIC)){
                            lcdPowerFunction((u8)argumentList[1].value);
                        }
                        //else{
                            //printf("\nRuntime execution error. Improper LCDP function call!");
                        //}
                        break;
                    case FUNCTION_VAR:
                            if(i == 0 && argumentList[1].exist){
                                    if(argumentList[3].exist){
                                variableFunction(argumentList[1].text, argumentList[3].value, &variableList, false);
                                            //printf("\nNew variable: %s = %u", argumentList[1].text, argumentList[3].value);
                                    }
                                    else{
                                variableFunction(argumentList[1].text, 0, &variableList, false);
                                        //printf("\nNew variable: %s, no init value", argumentList[1].text);
                                    }
                            }
                            else{
                                    //printf("\nRuntime execution error. Improper variable declaration!");
                                    goto endExecution;
                            }
                            break;
                    case FUNCTION_SPIR:
                        if(argumentList[0].type == ARGTYPE_VARIABLE && argumentList[1].exist){
                            arithAccumulator = SPIRead();
                            accumulatorInUse = true;
                        }
                        break;
                    case FUNCTION_SPIW:
                        if(argumentList[1].exist && (argumentList[1].type == ARGTYPE_VARIABLE || argumentList[1].type == ARGTYPE_NUMERIC)){
                            SPIWrite((u8)argumentList[1].value);
                        }
                        //else{
                            //printf("\nRuntime execution error. Improper LCDP function call!");
                        //}
                        break;
                    case FUNCTION_XPAD:
                        if(argumentList[0].type == ARGTYPE_VARIABLE && argumentList[1].exist){
                            arithAccumulator = XPADRead();
                            accumulatorInUse = true;
                        }
                        break;
                    case FUNCTION_END:
                        //printf("\nEND function. Ending script execution gracefully.");
                        goto endExecution;
                        break;
                    default:
                        //printf("\nUnknown function call. Halting!");
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
                                    //printf("\nRuntime execution error. Undeclared variable");
                                    goto endExecution;
                                }
                                //printf("\n%s = %u", argumentList[0].text, argumentList[0].value);
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
                                    case OPTYPE_SHIFTUP:
                                        arithAccumulator = argumentList[i].value << arithAccumulator;
                                        break;
                                    case OPTYPE_SHIFTDOWN:
                                        arithAccumulator = argumentList[i].value >> arithAccumulator;
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
                                case OPTYPE_SHIFTUP:
                                    arithAccumulator = argumentList[i].value << arithAccumulator;
                                    break;
                                case OPTYPE_SHIFTDOWN:
                                    arithAccumulator = argumentList[i].value >> arithAccumulator;
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
//printk("\n     endExecution reached");
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
	//printk("\n     if function called : %u %u %u",param1, op, param2);
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
    //printf("\n****GPI function called, return 1");
    LPCMod_ReadIO(NULL);
    if(port)
        return GenPurposeIOs.GPI1;
    return GenPurposeIOs.GPI0;
}
bool gpoFunction(u8 port, u8 value){
    //printf("\n****Set GPO port 0x%X with value 0x%X", port, value);
    LPCMod_WriteIO(port, value);
    return true;
}
bool waitFunction(int ms){
    wait_ms(ms);
    //printk("\n     wait function called : %ums",ms);
    return true;
}
bool bootFunction(u8 bank){
    //printf("\n****Boot bank: %u", bank);
	//printk("\n     boot function called : %u",bank);
    if(bank == BNK512 || bank == BNK256 || bank == BNKOS){
        BootModBios((void *)&bank);
    }
    else if(bank == BNKTSOPSPLIT0 || bank == BNKTSOPSPLIT1 || bank == BNKFULLTSOP){
        BootOriginalBios((void *)&bank);
    }
    else
        return false;
    return true;
}
bool fanFunction(u8 value){
    //printf("\n****Fan speed: %u", value);
	//printk("\n     fan function called : %u\%",value);
    if(value >=10 && value <= 100)
        I2CSetFanSpeed(value);
    return true;
}
bool ledFunction(char * value){
	//printk("\n     LED function called : %s", value);
    //printf("\n****LED pattern: %s", value);
    setLED(value);
    return true;
}
bool lcdPrintFunction(u8 line, char * text, u8 stringLength){
    //printf("\n****LCD Print at line %u : %s", line, text);
    //printk("\n     lcdPrint function called : %s",text);
    char tempString[xLCD.LineSize + 1];
    u8 inputStringLength;
    if(line > (xLCD.nbLines - 1))
        return false;

    if(stringLength < xLCD.LineSize){
        inputStringLength = strlen(text);
        if(inputStringLength < stringLength)
            stringLength = inputStringLength;

        strncpy(tempString, text, stringLength);
        tempString[stringLength] = '\0';
    }
    else{
    	strncpy(tempString, text, xLCD.LineSize);
    	tempString[xLCD.LineSize] = '\0';
    }

    BootLCDUpdateLinesOwnership(line, SCRIPT_OWNER);
    xLCD.PrintLine[line](0, tempString);      //0 for justify text on left

    return true;
}
bool lcdClearLineFunction(u8 line){
	//printk("\n     lcdClearLine function called : %u",line);
    //printf("\n****LCD clear line %u", line);
    if(line > (xLCD.nbLines - 1))
        return false;

    WriteLCDClearLine(line);
    return true;
}
bool lcdResetFunction(void){
    //printf("\n****LCD reset screen");
	//printk("\n     lcdReset function called");
    WriteLCDCommand(0x01);      //CLEAR command
    return true;
}
bool lcdBacklightFunction(u8 value){
	//printk("\n     lcdBacklight function called : %u\%",value);
    //printf("\n****LCD backlight value: %u", value);
    setLCDBacklight(value);
    return true;
}
bool lcdPowerFunction(u8 value){
	//printk("\n     lcdPower function called : %u",value);
    //printf("\n****LCD power %s", value ? "ON": "OFF");
    LPCmodSettings.LCDsettings.enable5V = value? 1 : 0;
    assertInitLCD();
    return true;
}

u8 SPIRead(void){
    u8 i, result = 0;
    for(i = 0; i < 8; i++){
        result = result << 1;
        LPCMod_WriteIO(0x2, 0);     //Reset CLK to 0
        //wait_us(1);
        LPCMod_WriteIO(0x2, 0x2);
        LPCMod_ReadIO(NULL);
        result |= GenPurposeIOs.GPI1 << i;
        //wait_us(1);    //This will need to be verified.
    }
    return result;
}

bool SPIWrite(u8 data){
    char i;
    for(i = 7; i >= 0; i--){
        //LPCMod_WriteIO(0x2, 0);     //Reset CLK to 0
        LPCMod_WriteIO(0x3, (data >> i)&0x01);
        //wait_us(1);
        LPCMod_WriteIO(0x2, 0x2);
        //wait_us(1);
    }
    LPCMod_WriteIO(0x3, 0);
    return true;
}

u8 XPADRead(void){
    //printf("\n****Controller Pad read");
    int i;
    for(i = TRIGGER_XPAD_KEY_A; i <= TRIGGER_XPAD_KEY_WHITE; i++){
        if(XPAD_current[0].keys[i])
            return (i +1);      //+1 to keep 0 value for no button pressed.
    }
    return 0;
}

bool variableFunction(char * name, int initValue, _variableList * variableList, bool readOnly){
    variableEntry * newEntry;
    int i;
    char * tempName = (char *)name;

    //printk("\n     variable function called : %s = %u",name, initValue);

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
    newEntry->readOnly = readOnly;

    variableList->last = newEntry;
    variableList->count += 1;

    return true;
}

void parseFileForLabels(u8 * file, u32 fileSize, _labelList * labelList){
    int stringStartPtr = 0, stringStopPtr = 0;
    char compareBuf[100];
    while(stringStopPtr < fileSize){      //We stay in file
        while(file[stringStopPtr] != '\n' && stringStopPtr < fileSize){        //While we don't hit a new line and still in file
            stringStopPtr++;        //Move on to next character in file.
        }
        if(file[stringStartPtr] == '$'){        //Label identifier detected
            //stringStartPtr is now a beginning of the line and stringStopPtr is at the end of it.
            //Copy line in compareBuf.
            strncpy(compareBuf, &file[stringStartPtr], stringStopPtr - stringStartPtr);
            //Manually append terminating character at the end of the string
            compareBuf[stringStopPtr - stringStartPtr] = '\0';
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
    char compareBuf[6];
    ifStatementEntry *newEntry;
    while(stringStopPtr < fileSize){      //We stay in file
        while(file[stringStopPtr] != '\n' && stringStopPtr < fileSize){        //While we don't hit a new line and still in file
            stringStopPtr++;        //Move on to next character in file.
        }
        stringTempPtr = stringStartPtr;
        while(file[stringTempPtr] != '(' && file[stringTempPtr] != '\n' && file[stringTempPtr] != '\0' && stringTempPtr < (stringStartPtr + 6))
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
                newEntry->validAssertion = false;        //Only useful for runtime
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
    while(compareBuf[tempPtr] != '\0' && tempPtr < 100){
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
        //printf("\nNew label found in script.\n  Name : %s\n  Points to : %u", newLabelEntry->name, newLabelEntry->stringPos);
    }
}

bool checkEndOfArgument(char * compareBuf, int position){

    //For string declaration, which must be in " "
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
    if(compareBuf[position] == '\0')
        return false;
    if(compareBuf[position] == '(')
        return false;
    if(compareBuf[position] == '#')
        return false;
    if(compareBuf[position] == ')')
        return false;
    if(compareBuf[position] == ',')
        return false;

    //Single character operators.
    if(compareBuf[position] == '+' || compareBuf[position] == '-'  || compareBuf[position] == '*' || compareBuf[position] == '/' || compareBuf[position] == '%')
        return false;

    //Cases where there is not space between operator and preceding argument
    if(position >= 1){
        if(compareBuf[position] == '!')
            return false;
        if(compareBuf[position] == '>' && compareBuf[position - 1] != '>')
            return false;
        if(compareBuf[position] == '<' && compareBuf[position - 1] != '<')
            return false;
        if(compareBuf[position] == '=' && compareBuf[position - 1] != '=' && compareBuf[position - 1] != '!' && compareBuf[position - 1] != '>' && compareBuf[position - 1] != '<')
            return false;
    }


    if(position >= 2){
        if((compareBuf[position - 2] == '>' && compareBuf[position - 1] == '>')  || (compareBuf[position - 2] == '<' && compareBuf[position - 1] == '<'))
            return false;
    }

    //Cases where there is not space between operator and following argument
    if(position >= 1){
        if(compareBuf[position - 1] == '+' || compareBuf[position - 1] == '-'  || compareBuf[position - 1] == '*' || compareBuf[position - 1] == '/' || compareBuf[position - 1] == '%')
            return false;

        if(compareBuf[position - 1] == '=' && compareBuf[position] != '=')
            return false;

        if(compareBuf[position - 1] == '>' && compareBuf[position] != '=' && compareBuf[position] != '>')
            return false;

        if(compareBuf[position - 1] == '<' && compareBuf[position] != '=' && compareBuf[position] != '<')
            return false;

    }

    if(position == 3)
        if(compareBuf[position - 3] == 'V' && compareBuf[position - 2] == 'A' && compareBuf[position - 1] == 'R')
            return false;
    if(position == 4)
        if(compareBuf[position - 4] == 'G' && compareBuf[position - 3] == 'O' && compareBuf[position - 2] == 'T' && compareBuf[position - 1] == 'O')
            return false;

    return true;
}

//Function return indicates what type of argument this is.
//-1 is unknown/skip
//0 is for function call
//1 is for operator
//2 is for variable call
//3 is for numerical value
//4 for string or
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
    if(!strcmp(inputArg, "<<")){
        *outNum = OPTYPE_SHIFTUP;
        operatorDetected = true;
    }
    if(!strcmp(inputArg, ">>")){
        *outNum = OPTYPE_SHIFTDOWN;
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
/*
        //Negative number?
        if(inputArg[0] == '-')
            i = 1;
        else
            i = 0;

        if(inputArg[i] >= '0' && inputArg[i] <= '9'){
            *outNum = atoi(&inputArg[i]);
            return ARGTYPE_NUMERIC;
        }
*/
        //Is it a number?
        if((inputArg[0] >= '0' && inputArg[0] <= '9') || (inputArg[0] == '0' && inputArg[1] == 'x') || (inputArg[0] == '-')){
			*outNum = strtol(inputArg, NULL, 0);
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

    return ARGTYPE_STRING;
}

bool updateVariable(char * name, int value, _variableList * variableList){
    variableEntry * currentEntry = variableList->first;
    int i;

    for(i = 0; i < variableList->count; i++){
        if(!strcmp(currentEntry->name, name)){
            if(!(currentEntry->readOnly))
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

int trimScript(u8 ** file, u32 fileSize){
    int newSize = 0, stringStartPtr = 0, stringStopPtr = 0;
    bool CRdetected;
    bool insideTextString;

    int linePtr, tempPtr;

    //Linked list of individual lines of text without space,
    //terminating with either new line character or terminating character for last line.
    typedef struct segment{
        char lineBuf[100];
        struct segment *nextSegment;
    } segment;

    segment *firstSegment = NULL;
    segment *currentSegment = NULL;


    while(stringStopPtr < fileSize){      //We stay in file
        while((*file)[stringStopPtr] != '\n' && stringStopPtr < fileSize){        //While we don't hit a new line and still in file
            stringStopPtr++;        //Move on to next character in file.
        }
        while(((*file)[stringStartPtr] == ' ' || (*file)[stringStartPtr] == '\t') && stringStartPtr < stringStopPtr)      //Skip leading spaces for indentation;
            stringStartPtr++;
        if((*file)[stringStartPtr] == '#' || stringStartPtr == stringStopPtr){       //This is a comment or empty line

            stringStartPtr = ++stringStopPtr;     //Prepare to move on to next line.
            continue;
        }

        CRdetected = (*file)[stringStopPtr - 1] == '\r' ? 1 : 0;    //Dos formatted line?
        linePtr = stringStartPtr;

        //We now have a complete line, that is not a comment, without leading space or tab and Carriage return character marked if present.
        //We need to remove all space characters

        //If first element of list is already present
        if(firstSegment != NULL){
            currentSegment->nextSegment = (segment *)malloc(sizeof(segment));
            //Move on the the next (empty) element
            currentSegment = currentSegment->nextSegment;
            currentSegment->nextSegment = NULL;
        }
        else{
        	//Create first element and point to it.
            firstSegment = (segment *)malloc(sizeof(segment));
            firstSegment->nextSegment = NULL;
            currentSegment = firstSegment;
        }

        //It's a new line
        tempPtr = 0;
        insideTextString = false;
        //While we don't hit end of line or comment identifier
        while(linePtr < (stringStopPtr - CRdetected) && (*file)[linePtr] != '#'){
        	if((*file)[linePtr] == '\"')
        		insideTextString = !insideTextString;
            if((*file)[linePtr] != ' ' || insideTextString){
            	//Copy into list element character by character as long as it's not a space.
                currentSegment->lineBuf[tempPtr] = (*file)[linePtr];
                tempPtr += 1;
            }
            linePtr += 1;
        }
        //Append at end of new line
        currentSegment->lineBuf[tempPtr] = '\n';
        //Move on to new line in the original buffer.
        stringStartPtr = ++stringStopPtr;

    }

    //All lines of the files have been copied in linked list
    tempPtr = 0;
    //currentSegment points to last line
    while(currentSegment->lineBuf[tempPtr] != '\n')
        tempPtr += 1;
    //We replace new line character by terminating.
    currentSegment->lineBuf[tempPtr] = '\0';
    //Move back to start of list
    currentSegment = firstSegment;
    //Count required bytes for new buffer by counting every character of every element in the list
    while(currentSegment != NULL){
        tempPtr = 0;
        while(currentSegment->lineBuf[tempPtr] != '\n' && currentSegment->lineBuf[tempPtr] != '\0'){
            tempPtr += 1;
        }
        newSize += (tempPtr + 1);
        currentSegment = currentSegment->nextSegment;
    }

    //we know the new size of the buffer.
    free(*file);
    *file = (u8*)malloc(newSize);

    //Move back to the start of the list.
    currentSegment = firstSegment;
    tempPtr = 0;
    //Copy every line of every element of the list in the new buffer.
    while(currentSegment != NULL){
        linePtr = 0;
        while(currentSegment->lineBuf[linePtr] != '\n' && currentSegment->lineBuf[linePtr] != '\0'){
            (*file)[tempPtr] = currentSegment->lineBuf[linePtr];
            linePtr += 1;
            tempPtr += 1;
        }
        (*file)[tempPtr] = currentSegment->lineBuf[linePtr];
        tempPtr += 1;
        currentSegment = currentSegment->nextSegment;
    }

    //Just to get in the loop
    currentSegment = firstSegment;
    //Delete malloc list elements
    while(currentSegment != NULL){
    	firstSegment = currentSegment;
    	currentSegment = currentSegment->nextSegment;
    	free(firstSegment);
    }

    return newSize;
}
