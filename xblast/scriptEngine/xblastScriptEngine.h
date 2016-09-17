#ifndef _XBLASTSCRIPTENGINE_H_
#define _XBLASTSCRIPTENGINE_H_

unsigned char * scriptSavingPtr;
unsigned char * bootScriptBuffer;

void runScript(unsigned char * file, unsigned int fileSize, int paramCount, int * param);
int trimScript(unsigned char ** file, unsigned int fileSize);

#endif // _XBLASTSCRIPTENGINE_H_
