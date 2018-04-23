/*  Glue functions for the minIni library, based on the FatFs and Petit-FatFs
 *  libraries, see http://elm-chan.org/fsw/ff/00index_e.html
 *
 *  By CompuPhase, 2008-2012
 *  This "glue file" is in the public domain. It is distributed without
 *  warranties or conditions of any kind, either express or implied.
 *
 *  (The FatFs and Petit-FatFs libraries are copyright by ChaN and licensed at
 *  its own terms.)
 */

#define INI_BUFFERSIZE  256       /* maximum line length, maximum path length */
#define INI_ANSIONLY /* ignore UNICODE or _UNICODE macros, compile as ASCII/ANSI */
#define INI_LINETERM "\r\n" /* For Windows users */
#define NDEBUG      /* No Assert macro */

/* You must set _USE_STRFUNC to 1 or 2 in the include file ff.h (or tff.h)
 * to enable the "string functions" fgets() and fputs().
 */
#include "FatFSAccessor.h"                   /* include tff.h for Tiny-FatFs */

#define INI_FILETYPE    FILEX
#define ini_openread(filename,file)   (0 != (*(file) = fatxopen(filename, FileOpenMode_Read | FileOpenMode_OpenExistingOnly)))
#define ini_openwrite(filename,file)  (0 != (*(file) = fatxopen(filename, FileOpenMode_Write | FileOpenMode_CreateAlways)))
#define ini_close(file)               (0 == fatxclose(*(file)))
#define ini_read(buffer,size,file)    (0 != (fatxgets((buffer), (size), *(file))))
#define ini_write(buffer,file)        (0 <= (fatxputs((buffer), *(file))))
#define ini_rename(source,dest)       (0 == fatxrename((source), (dest)))
#define ini_remove(filename)          (0 == (fatxdelete(filename)))

#define INI_FILEPOS                   unsigned long
#define ini_tell(file,pos)            (*(pos) = fatxtell(*(file)))
#define ini_seek(file,pos)            (0 == fatxseek(*(file), *(pos)))

