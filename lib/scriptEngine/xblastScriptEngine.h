#ifndef _XBLASTSCRIPTENGINE_H_
#define _XBLASTSCRIPTENGINE_H_

void runScript(u8 * file, u32 fileSize, int paramCount, int * param);
int trimScript(u8 ** file, u32 fileSize);

#endif // _XBLASTSCRIPTENGINE_H_
