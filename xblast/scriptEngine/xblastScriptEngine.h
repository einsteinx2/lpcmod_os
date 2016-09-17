#ifndef _XBLASTSCRIPTENGINE_H_
#define _XBLASTSCRIPTENGINE_H_

<<<<<<< HEAD
unsigned char * scriptSavingPtr;
unsigned char * bootScriptBuffer;

void runScript(unsigned char * file, unsigned int fileSize, int paramCount, int * param);
int trimScript(unsigned char ** file, unsigned int fileSize);
=======
void runScript(u8 * file, u32 fileSize, int paramCount, int * param);
int trimScript(u8 ** file, u32 fileSize);
>>>>>>> branch 'master' of https://psyko_chewbacca@bitbucket.org/psyko_chewbacca/lpcmod_os.git

#endif // _XBLASTSCRIPTENGINE_H_
