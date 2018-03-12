//============================================================================
// Name        : fatfs_test.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include "ff.h"
#include "diskio.h"
#include <iostream>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <cstring>

/*---------------------------------------------------------*/
/* Work Area                                               */
/*---------------------------------------------------------*/

LONGLONG AccSize;           /* Work register for scan_files() */
WORD AccFiles, AccDirs;
FILINFO Finfo;

TCHAR Line[300];            /* Console input/output buffer */
HANDLE hCon, hKey;

FATFS FatFs[_VOLUMES];      /* File system object for logical drive */
BYTE Buff[262144];          /* Working buffer */

#if _USE_FASTSEEK
DWORD SeekTbl[16];          /* Link map table for fast seek feature */
#endif

PARTITION VolToPart[] =
{
 {0, 0}, /* XBOX SHELL, E:\ */
 {0, 1}, /* XBOX DATA, C:\ */
 {0, 2}, /* XBOX GAME SWAP 1, X:\ */
 {0, 3}, /* XBOX GAME SWAP 2, Y:\ */
 {0, 4}, /* XBOX GAME SWAP 3, Z:\ */
 {0, 5}, /* XBOX F, F:\ */
 {0, 6}, /* XBOX G, G:\ */
 {1, 0}, /* XBOX SHELL, E:\ */
 {1, 1}, /* XBOX DATA, C:\ */
 {1, 2}, /* XBOX GAME SWAP 1, X:\ */
 {1, 3}, /* XBOX GAME SWAP 2, Y:\ */
 {1, 4}, /* XBOX GAME SWAP 3, Z:\ */
 {1, 5}, /* XBOX F, F:\ */
 {1, 6}  /* XBOX G, G:\ */
};

const char* DefaultImageFilename = "XboxImage.bin";

using namespace std;

unsigned int get_fattime (void)
{
    time_t outTime;
    time(&outTime);
    return (unsigned int)outTime;
}

/*--------------------------------------------------------------------------*/
/* Monitor                                                                  */

/*----------------------------------------------*/
/* Get a value of the string                    */
/*----------------------------------------------*/
/*  "123 -5   0x3ff 0b1111 0377  w "
        ^                           1st call returns 123 and next ptr
           ^                        2nd call returns -5 and next ptr
                   ^                3rd call returns 1023 and next ptr
                          ^         4th call returns 15 and next ptr
                               ^    5th call returns 255 and next ptr
                                  ^ 6th call fails and returns 0
*/

int xatoll (        /* 0:Failed, 1:Successful */
    TCHAR **str,    /* Pointer to pointer to the string */
    QWORD *res      /* Pointer to a valiable to store the value */
)
{
    QWORD val;
    unsigned char r;
    TCHAR c;


    *res = 0;
    while ((c = **str) == ' ') (*str)++;    /* Skip leading spaces */

    if (c == '0') {
        c = *(++(*str));
        switch (c) {
        case 'x':       /* hexdecimal */
            r = 16; c = *(++(*str));
            break;
        case 'b':       /* binary */
            r = 2; c = *(++(*str));
            break;
        default:
            if (c <= ' ') return 1; /* single zero */
            if (c < '0' || c > '9') return 0;   /* invalid char */
            r = 8;      /* octal */
        }
    } else {
        if (c < '0' || c > '9') return 0;   /* EOL or invalid char */
        r = 10;         /* decimal */
    }

    val = 0;
    while (c > ' ') {
        if (c >= 'a') c -= 0x20;
        c -= '0';
        if (c >= 17) {
            c -= 7;
            if (c <= 9) return 0;   /* invalid char */
        }
        if (c >= r) return 0;       /* invalid char for current radix */
        val = val * r + c;
        c = *(++(*str));
    }

    *res = val;
    return 1;
}


int xatoi (
    TCHAR **str,    /* Pointer to pointer to the string */
    DWORD *res      /* Pointer to a valiable to store the value */
)
{
    QWORD d;


    *res = 0;
    if (!xatoll(str, &d)) return 0;
    *res = (DWORD)d;
    return 1;
}




/*----------------------------------------------*/
/* Dump a block of byte array                   */

void put_dump (
    const unsigned char* buff,  /* Pointer to the byte array to be dumped */
    unsigned long addr,         /* Heading address value */
    int cnt                     /* Number of bytes to be dumped */
)
{
    int i;


    printf(_T("%08lX:"), addr);

    for (i = 0; i < cnt; i++)
        printf(_T(" %02X"), buff[i]);

    putchar(' ');
    for (i = 0; i < cnt; i++)
        putchar((TCHAR)((buff[i] >= ' ' && buff[i] <= '~') ? buff[i] : '.'));

    putchar('\n');
}



UINT forward (
    const BYTE* buf,
    UINT btf
)
{
    UINT i;


    if (btf) {  /* Transfer call? */
        for (i = 0; i < btf; i++) putchar(buf[i]);
        return btf;
    } else {    /* Sens call */
        return 1;
    }
}



FRESULT scan_files (
    TCHAR* path     /* Pointer to the path name working buffer */
)
{
    DIR dir;
    FRESULT res;
    int i;


    if ((res = f_opendir(&dir, path)) == FR_OK) {
        i = strlen(path);
        while (((res = f_readdir(&dir, &Finfo)) == FR_OK) && Finfo.fnamex[0]) {
            if (_FS_RPATH && Finfo.fnamex[0] == '.') continue;
            if (Finfo.fattrib & AM_DIR) {
                AccDirs++;
                *(path+i) = '/'; strcpy(path+i+1, Finfo.fnamex);
                res = scan_files(path);
                *(path+i) = '\0';
                if (res != FR_OK) break;
            } else {
//              printf(_T("%s/%s\n"), path, fn);
                AccFiles++;
                AccSize += Finfo.fsize;
            }
        }
        f_closedir(&dir);
    }

    return res;
}



void put_rc (FRESULT rc)
{
    const TCHAR *p =
        _T("OK\0DISK_ERR\0INT_ERR\0NOT_READY\0NO_FILE\0NO_PATH\0INVALID_NAME\0")
        _T("DENIED\0EXIST\0INVALID_OBJECT\0WRITE_PROTECTED\0INVALID_DRIVE\0")
        _T("NOT_ENABLED\0NO_FILE_SYSTEM\0MKFS_ABORTED\0TIMEOUT\0LOCKED\0")
        _T("NOT_ENOUGH_CORE\0TOO_MANY_OPEN_FILES\0INVALID_PARAMETER\0");
    int i;

    for (i = 0; i != rc && *p; i++) {
        while(*p++) ;
    }
    printf(_T("rc=%u FR_%s\n"), (UINT)rc, p);
}



const TCHAR HelpStr[] = {
        _T("[Disk contorls]\n")
        _T(" di <pd#> - Initialize disk\n")
        _T(" dd [<pd#> <sect>] - Dump a secrtor\n")
        _T(" ds <pd#> - Show disk status\n")
        _T(" dl <file> - Load FAT image into RAM disk (pd#0)\n")
        _T("[Buffer contorls]\n")
        _T(" bd <ofs> - Dump working buffer\n")
        _T(" be <ofs> [<data>] ... - Edit working buffer\n")
        _T(" br <pd#> <sect> <count> - Read disk into working buffer\n")
        _T(" bw <pd#> <sect> <count> - Write working buffer into disk\n")
        _T(" bf <val> - Fill working buffer\n")
        _T("[File system controls]\n")
        _T(" fi <ld#> [<opt>] - Force initialized the volume\n")
        _T(" fs [<path>] - Show volume status\n")
        _T(" fl [<path>] - Show a directory\n")
        _T(" fL <path> <pat> - Find a directory\n")
        _T(" fo <mode> <file> - Open a file\n")
        _T(" fc - Close the file\n")
        _T(" fe <ofs> - Move fp in normal seek\n")
        _T(" fE <ofs> - Move fp in fast seek or Create link table\n")
        _T(" ff <len> - Forward file data to the console\n")
        _T(" fh <fsz> <opt> - Allocate a contiguous block to the file\n")
        _T(" fd <len> - Read and dump the file\n")
        _T(" fr <len> - Read the file\n")
        _T(" fw <len> <val> - Write to the file\n")
        _T(" fn <object name> <new name> - Rename an object\n")
        _T(" fu <object name> - Unlink an object\n")
        _T(" fv - Truncate the file at current fp\n")
        _T(" fk <dir name> - Create a directory\n")
        _T(" fa <atrr> <mask> <object name> - Change object attribute\n")
        _T(" ft <year> <month> <day> <hour> <min> <sec> <object name> - Change timestamp of an object\n")
        _T(" fx <src file> <dst file> - Copy a file\n")
        _T(" fg <path> - Change current directory\n")
        _T(" fj <path> - Change current drive\n")
        _T(" fq - Show current directory path\n")
        _T(" fb <name> - Set volume label\n")
        _T(" fm <ld#> <type> <cluster size> - Create file system\n")
        _T(" fp <pd#> <p1 size> <p2 size> <p3 size> <p4 size> - Divide physical drive\n")
        _T("\n")
    };

void get_uni (
    TCHAR* buf,
    UINT len
)
{
/*
    UINT i = 0;
    DWORD n;

    for (;;) {
        ReadConsole(hKey, &buf[i], 1, &n, 0);
        if (buf[i] == 8) {
            if (i) i--;
            continue;
        }
        if (buf[i] == 13) {
            buf[i] = 0;
            break;
        }
        if ((UINT)buf[i] >= ' ' && i + n < len) i += n;
    }
    */
    fgets(buf, len, stdin);
}


void put_uni (
    TCHAR* buf
)
{
    fputs(buf, stdout);
}


int main(int argc, TCHAR *argv[]) {
    TCHAR *ptr, *ptr2, pool[50];
    DWORD p1, p2, p3;
    QWORD px;
    BYTE *buf;
    UINT s1, s2, cnt;
    WORD w;
    DWORD dw, ofs = 0, sect = 0, drv = 0;
    const TCHAR *ft[] = {_T("N/A"), _T("FAT12"), _T("FAT16"), _T("FAT32"), _T("exFAT"), _T("FATX16"), _T("FATX32")};
    FILE* h;
    FRESULT fr;
    DRESULT dr;
    FATFS *fs;              /* Pointer to file system object */
    DIR dir;                /* Directory object */
    FIL file[2];            /* File objects */
    const char* filename = DefaultImageFilename;

    if(argc > 2)
    {
        printf("Too many arguments\n");
    }
    else if(argc ==  2)
    {
        if(argv[1])
        {
            filename = argv[1];
        }
    }
    printf("Opening \"%s\" disk image\n", filename);

    printf(_T("FatFs module test monitor (%s, CP:%u/%s)\n\n"),
                _USE_LFN ? _T("LFN") : _T("SFN"),
                _CODE_PAGE,
                _LFN_UNICODE ? _T("Unicode") : _T("ANSI"));

    printf(pool, _T("FatFs debug console (%s, CP:%u/%s)"),
                _USE_LFN ? _T("LFN") : _T("SFN"),
                _CODE_PAGE,
                _LFN_UNICODE ? _T("Unicode") : _T("ANSI"));

    if(assign_drives(filename))    /* Find physical drives on the PC */
    {
        printf("Disk image not found, exiting.\n");
        return EXIT_FAILURE;
    }

#if _MULTI_PARTITION
    printf(_T("\nMultiple partition is enabled. Each logical drive is tied to the patition as follows:\n"));
    for (cnt = 0; cnt < sizeof VolToPart / sizeof (PARTITION); cnt++) {
        const TCHAR *pn[] = {_VOLUME_STRS};

        printf(_T("\"%u:\" <== Disk# %u, %s\n"), cnt, VolToPart[cnt].pd, pn[(NbFATXPartPerHDD * VolToPart[cnt].pd) + VolToPart[cnt].pt]);
    }
    printf(_T("\n"));
#else
    printf(_T("\nMultiple partition is disabled.\nEach logical drive is tied to the same physical drive number.\n\n"));
#endif

    for (;;) {
            printf(_T(">"));
            get_uni(Line, sizeof Line / sizeof *Line);
            ptr = Line;

            switch (*ptr++) {   /* Branch by primary command character */

            case 'q' :  /* Exit program */
                return 0;

            case '?':       /* Show usage */
                printf(HelpStr);
                break;

            case 'T' :

                /* Quick test space */

                break;

            case 'd' :  /* Disk I/O command */
                switch (*ptr++) {   /* Branch by secondary command character */
                case 'd' :  /* dd [<pd#> <sect>] - Dump a secrtor */
                    if (!xatoi(&ptr, &p1)) {
                        p1 = drv; p2 = sect;
                    } else {
                        if (!xatoi(&ptr, &p2)) break;
                    }
                    dr = disk_read((BYTE)p1, Buff, p2, 1);
                    if (dr) { printf(_T("rc=%d\n"), (WORD)dr); break; }
                    printf(_T("Drive:%lu Sector:%lu\n"), p1, p2);
                    if (disk_ioctl((BYTE)p1, GET_SECTOR_SIZE, &w) != RES_OK) break;
                    sect = p2 + 1; drv = p1;
                    for (buf = Buff, ofs = 0; ofs < w; buf += 16, ofs += 16)
                        put_dump(buf, ofs, 16);
                    break;

                case 'i' :  /* di <pd#> - Initialize physical drive */
                    if (!xatoi(&ptr, &p1)) break;
                    dr = disk_initialize((BYTE)p1);
                    printf(_T("rc=%d\n"), dr);
                    if (disk_ioctl((BYTE)p1, GET_SECTOR_SIZE, &w) == RES_OK)
                        printf(_T("Sector size = %u\n"), w);
                    if (disk_ioctl((BYTE)p1, GET_SECTOR_COUNT, &dw) == RES_OK)
                        printf(_T("Number of sectors = %lu\n"), dw);
                    break;

                case 'l' :  /* dl <image file> - Load image of a FAT volume into RAM disk */
                    while (*ptr == ' ') ptr++;
                    if (disk_ioctl(0, 200, ptr) == RES_OK)
                        printf(_T("Ok\n"));
                    break;

                }
                break;

            case 'b' :  /* Buffer control command */
                switch (*ptr++) {   /* Branch by secondary command character */
                case 'd' :  /* bd <ofs> - Dump Buff[] */
                    if (!xatoi(&ptr, &p1)) break;
                    for (buf = &Buff[p1], ofs = p1, cnt = 32; cnt; cnt--, buf += 16, ofs += 16)
                        put_dump(buf, ofs, 16);
                    break;

                case 'e' :  /* be <ofs> [<data>] ... - Edit Buff[] */
                    if (!xatoi(&ptr, &p1)) break;
                    if (xatoi(&ptr, &p2)) {
                        do {
                            Buff[p1++] = (BYTE)p2;
                        } while (xatoi(&ptr, &p2));
                        break;
                    }
                    for (;;) {
                        printf(_T("%04X %02X-"), (WORD)(p1), (WORD)Buff[p1]);
                        get_uni(Line, sizeof Line / sizeof *Line);
                        ptr = Line;
                        if (*ptr == '.') break;
                        if (*ptr < ' ') { p1++; continue; }
                        if (xatoi(&ptr, &p2))
                            Buff[p1++] = (BYTE)p2;
                        else
                            printf(_T("???\n"));
                    }
                    break;

                case 'r' :  /* br <pd#> <sector> <count> - Read disk into Buff[] */
                    if (!xatoi(&ptr, &p1) || !xatoi(&ptr, &p2) || !xatoi(&ptr, &p3)) break;
                    printf(_T("rc=%u\n"), disk_read((BYTE)p1, Buff, p2, p3));
                    break;

                case 'w' :  /* bw <pd#> <sect> <count> - Write Buff[] into disk */
                    if (!xatoi(&ptr, &p1) || !xatoi(&ptr, &p2) || !xatoi(&ptr, &p3)) break;
                    printf(_T("rc=%u\n"), disk_write((BYTE)p1, Buff, p2, p3));
                    break;

                case 'f' :  /* bf <n> - Fill Buff[] */
                    if (!xatoi(&ptr, &p1)) break;
                    memset(Buff, p1, sizeof Buff);
                    break;

                case 's' :  /* bs - Save Buff[] */
                    while (*ptr == ' ') ptr++;
                    h = fopen ("Buff.bin","w");
                    if (h) {
                        fputs((const char *)Buff, h);
                        fclose(h);
                        printf(_T("Ok.\n"));
                    }

                    break;

                }
                break;

            case 'f' :  /* FatFs test command */
                switch (*ptr++) {   /* Branch by secondary command character */

                case 'i' :  /* fi <ld#> [<mount>] - Force initialized the logical drive */
                    if (!xatoi(&ptr, &p1) || (UINT)p1 > 9) break;
                    if (!xatoi(&ptr, &p2)) p2 = 0;
                    sprintf(ptr, _T("%lu:"), p1);
                    put_rc(f_mount(&FatFs[p1], ptr, (BYTE)p2));
                    break;

                case 's' :  /* fs [<path>] - Show logical drive status */
                    while (*ptr == ' ') ptr++;
                    ptr2 = ptr;
    #if _FS_READONLY
                    fr = f_opendir(&dir, ptr);
                    if (fr == FR_OK) {
                        fs = dir.obj.fs;
                        f_closedir(&dir);
                    }
    #else
                    fr = f_getfree(ptr, (DWORD*)&p1, &fs);
    #endif
                    if (fr) { put_rc(fr); break; }
                    printf(_T("FAT type = %s\n"), ft[fs->fs_typex]);
                    printf(_T("Cluster size = %lu bytes\n"),
    #if _MAX_SS != _MIN_SS
                        (DWORD)fs->csize * fs->ssize);
    #else
                        (DWORD)fs->csize * _MAX_SS);
    #endif
                    if (fs->fs_typex < FS_FAT32) printf(_T("Root DIR entries = %u\n"), fs->n_rootdir);
                    printf(_T("Sectors/FAT = %lu\nNumber of FATs = %u\nNumber of clusters = %lu\nVolume start sector = %lu\nFAT start sector = %lu\nRoot DIR start %s = %lu\nData start sector = %lu\n\n"),
                        fs->fsize, fs->n_fats, fs->n_fatent - 2, fs->volbase, fs->fatbase, fs->fs_typex != FS_EXFAT ? _T("cluster") : _T("sector"), fs->dirbase, fs->database);
    #if _USE_LABEL
                    fr = f_getlabel(ptr2, pool, &dw);
                    if (fr) { put_rc(fr); break; }
                    if (pool[0]) {
                        printf(_T("Volume name is "));
                        put_uni(pool);
                        printf(_T("\n"));
                    } else {
                        printf(_T("No volume label\n"));
                    }
                    printf(_T("Volume S/N is %04X-%04X\n"), dw >> 16, dw & 0xFFFF);
    #endif
                    printf(_T("..."));
                    AccSize = AccFiles = AccDirs = 0;
                    fr = scan_files(ptr);
                    if (fr) { put_rc(fr); break; }
                    p2 = (fs->n_fatent - 2) * fs->csize;
                    p3 = p1 * fs->csize;
    #if _MAX_SS != _MIN_SS
                    p2 *= fs->ssize / 512;
                    p3 *= fs->ssize / 512;
    #endif
                    p2 /= 2;
                    p3 /= 2;
                    printf(_T("\r%u files, %llu bytes.\n%u folders.\n%lu KiB total disk space.\n"),
                            AccFiles, AccSize, AccDirs, p2);
    #if !_FS_READONLY
                    printf(_T("%lu KiB available.\n"), p3);
    #endif
                    break;

                case 'l' :  /* fl [<path>] - Directory listing */
                    while (*ptr == ' ') ptr++;
                    fr = f_opendir(&dir, ptr);
                    if (fr) { put_rc(fr); break; }
                    AccSize = s1 = s2 = 0;
                    for(;;) {
                        fr = f_readdir(&dir, &Finfo);
                        if ((fr != FR_OK) || !Finfo.fnamex[0]) break;
                        if (Finfo.fattrib & AM_DIR) {
                            s2++;
                        } else {
                            s1++; AccSize += Finfo.fsize;
                        }
                        printf(_T("%c%c%c%c%c %u/%02u/%02u %02u:%02u %10llu  "),
                                (Finfo.fattrib & AM_DIR) ? 'D' : '-',
                                (Finfo.fattrib & AM_RDO) ? 'R' : '-',
                                (Finfo.fattrib & AM_HID) ? 'H' : '-',
                                (Finfo.fattrib & AM_SYS) ? 'S' : '-',
                                (Finfo.fattrib & AM_ARC) ? 'A' : '-',
                                (Finfo.fdate >> 9) + 1980, (Finfo.fdate >> 5) & 15, Finfo.fdate & 31,
                                (Finfo.ftime >> 11), (Finfo.ftime >> 5) & 63, (QWORD)Finfo.fsize);
    #if _USE_LFN && _USE_FIND == 2
                        printf(_T("%-12s  "),Finfo.altname);
    #endif
                        put_uni(Finfo.fnamex);
                        printf(_T("\n"));
                    }
                    f_closedir(&dir);
                    printf(_T("%4u File(s),%11llu bytes total\n%4u Dir(s)"), s1, AccSize, s2);
    #if !_FS_READONLY
                    if (f_getfree(ptr, (DWORD*)&p1, &fs) == FR_OK)
                        printf(_T(",%12llu bytes free"), (QWORD)p1 * fs->csize * 512);
    #endif
                    printf(_T("\n"));
                    break;
    #if _USE_FIND
                case 'L' :  /* fL <path> <pattern> - Directory search */
                    while (*ptr == ' ') ptr++;
                    ptr2 = ptr;
                    while (*ptr != ' ') ptr++;
                    *ptr++ = 0;
                    fr = f_findfirst(&dir, &Finfo, ptr2, ptr);
                    while (fr == FR_OK && Finfo.fnamex[0]) {
    #if _USE_LFN && _USE_FIND == 2
                        printf(_T("%-12s  "), Finfo.altname);
    #endif
                        put_uni(Finfo.fnamex);
                        printf(_T("\n"));
                        fr = f_findnext(&dir, &Finfo);
                    }
                    if (fr) put_rc(fr);

                    f_closedir(&dir);
                    break;
    #endif
                case 'o' :  /* fo <mode> <file> - Open a file */
                    if (!xatoi(&ptr, &p1)) break;
                    while (*ptr == ' ') ptr++;
                    fr = f_open(file, ptr, (BYTE)p1);
                    put_rc(fr);
                    break;

                case 'c' :  /* fc - Close a file */
                    put_rc(f_close(file));
                    break;

                case 'r' :  /* fr <len> - read file */
                    if (!xatoi(&ptr, &p1)) break;
                    p2 =0;
                    while (p1) {
                        if (p1 >= sizeof Buff) {
                            cnt = sizeof Buff; p1 -= sizeof Buff;
                        } else {
                            cnt = p1; p1 = 0;
                        }
                        fr = f_read(file, Buff, cnt, &s2);
                        if (fr != FR_OK) { put_rc(fr); break; }
                        p2 += s2;
                        if (cnt != s2) break;
                    }
                    printf(_T("%lu bytes read.\n"), p2);
                    break;

                case 'd' :  /* fd <len> - read and dump file from current fp */
                    if (!xatoi(&ptr, &p1)) p1 = 128;
                    ofs = (DWORD)file[0].fptr;
                    while (p1) {
                        if (p1 >= 16) { cnt = 16; p1 -= 16; }
                        else          { cnt = p1; p1 = 0; }
                        fr = f_read(file, Buff, cnt, &cnt);
                        if (fr != FR_OK) { put_rc(fr); break; }
                        if (!cnt) break;
                        put_dump(Buff, ofs, cnt);
                        ofs += 16;
                    }
                    break;

                case 'e' :  /* fe <ofs> - Seek file pointer */
                    if (!xatoll(&ptr, &px)) break;
                    fr = f_lseek(file, (FSIZE_t)px);
                    put_rc(fr);
                    if (fr == FR_OK)
                        printf(_T("fptr = %llu(0x%llX)\n"), (QWORD)file[0].fptr, (QWORD)file[0].fptr);
                    break;
    #if _USE_FASTSEEK
                case 'E' :  /* fE - Enable fast seek and initialize cluster link map table */
                    file[0].cltbl = SeekTbl;            /* Enable fast seek (set address of buffer) */
                    SeekTbl[0] = sizeof SeekTbl / sizeof *SeekTbl;  /* Buffer size */
                    fr = f_lseek(file, CREATE_LINKMAP); /* Create link map table */
                    put_rc(fr);
                    if (fr == FR_OK) {
                        printf((SeekTbl[0] > 4) ? _T("fragmented in %lu.\n") : _T("contiguous.\n"), SeekTbl[0] / 2 - 1);
                        printf(_T("%lu items used.\n"), SeekTbl[0]);

                    }
                    if (fr == FR_NOT_ENOUGH_CORE) {
                        printf(_T("%lu items required to create the link map table.\n"), SeekTbl[0]);
                    }
                    break;
    #endif  /* _USE_FASTSEEK */
    #if _FS_RPATH >= 1
                case 'g' :  /* fg <path> - Change current directory */
                    while (*ptr == ' ') ptr++;
                    put_rc(f_chdir(ptr));
                    break;
    #if _VOLUMES >= 2
                case 'j' :  /* fj <path> - Change current drive */
                    while (*ptr == ' ') ptr++;
                    put_rc(f_chdrive(ptr));
                    break;
    #endif
    #if _FS_RPATH >= 2
                case 'q' :  /* fq - Show current dir path */
                    fr = f_getcwd(Line, 256);
                    if (fr) {
                        put_rc(fr);
                    } else {
                        printf("%s", Line);
                        printf(_T("\n"));
                    }
                    break;
    #endif
    #endif
    #if _USE_FORWARD
                case 'f' :  /* ff <len> - Forward data */
                    if (!xatoi(&ptr, &p1)) break;
                    fr = f_forward(file, forward, p1, &s1);
                    put_rc(fr);
                    if (fr == FR_OK) printf(_T("\n%u bytes tranferred.\n"), s1);
                    break;
    #endif
    #if !_FS_READONLY
                case 'w' :  /* fw <len> <val> - Write file */
                    if (!xatoi(&ptr, &p1) || !xatoi(&ptr, &p2)) break;
                    memset(Buff, p2, sizeof Buff);
                    p2 = 0;
                    while (p1) {
                        if (p1 >= sizeof Buff) { cnt = sizeof Buff; p1 -= sizeof Buff; }
                        else                  { cnt = p1; p1 = 0; }
                        fr = f_write(file, Buff, cnt, &s2);
                        if (fr != FR_OK) { put_rc(fr); break; }
                        p2 += s2;
                        if (cnt != s2) break;
                    }
                    printf(_T("%lu bytes written.\n"), p2);
                    break;

                case 'v' :  /* fv - Truncate file */
                    put_rc(f_truncate(file));
                    break;

                case 'n' :  /* fn <name> <new_name> - Change file/dir name */
                    while (*ptr == ' ') ptr++;
                    ptr2 = strchr(ptr, ' ');
                    if (!ptr2) break;
                    *ptr2++ = 0;
                    while (*ptr2 == ' ') ptr2++;
                    put_rc(f_rename(ptr, ptr2));
                    break;

                case 'u' :  /* fu <name> - Unlink a file/dir */
                    while (*ptr == ' ') ptr++;
                    put_rc(f_unlink(ptr));
                    break;

                case 'k' :  /* fk <name> - Create a directory */
                    while (*ptr == ' ') ptr++;
                    put_rc(f_mkdir(ptr));
                    break;

                case 'x' : /* fx <src_name> <dst_name> - Copy a file */
                    while (*ptr == ' ') ptr++;
                    ptr2 = strchr(ptr, ' ');
                    if (!ptr2) break;
                    *ptr2++ = 0;
                    while (*ptr2 == ' ') ptr2++;
                    printf(_T("Opening \"%s\""), ptr);
                    fr = f_open(file + 0, ptr, FA_OPEN_EXISTING | FA_READ);
                    printf(_T("\n"));
                    if (fr) {
                        put_rc(fr);
                        break;
                    }
                    while (*ptr2 == ' ') ptr2++;
                    printf(_T("Creating \"%s\""), ptr2);
                    fr = f_open(file + 1, ptr2, FA_CREATE_ALWAYS | FA_WRITE);
                    printf(_T("\n"));
                    if (fr) {
                        put_rc(fr);
                        f_close(file + 0);
                        break;
                    }
                    printf(_T("Copying..."));
                    p1 = 0;
                    for (;;) {
                        fr = f_read(file + 0, Buff, sizeof Buff, &s1);
                        if (fr || s1 == 0) break;   /* error or eof */
                        fr = f_write(file + 1, Buff, s1, &s2);
                        p1 += s2;
                        if (fr || s2 < s1) break;   /* error or disk full */
                    }
                    printf(_T("\n"));
                    if (fr) put_rc(fr);
                    f_close(file + 0);
                    f_close(file + 1);
                    printf(_T("%lu bytes copied.\n"), p1);
                    break;
    #if _USE_EXPAND
                case 'h':   /* fh <fsz> <opt> - Allocate contiguous block */
                    if (!xatoll(&ptr, &px) || !xatoi(&ptr, &p2)) break;
                    fr = f_expand(file, (FSIZE_t)px, (BYTE)p2);
                    put_rc(fr);
                    break;
    #endif
    #if _USE_CHMOD
                case 'a' :  /* fa <atrr> <mask> <name> - Change file/dir attribute */
                    if (!xatoi(&ptr, &p1) || !xatoi(&ptr, &p2)) break;
                    while (*ptr == ' ') ptr++;
                    put_rc(f_chmod(ptr, (BYTE)p1, (BYTE)p2));
                    break;

                case 't' :  /* ft <year> <month> <day> <hour> <min> <sec> <name> - Change timestamp of a file/dir */
                    if (!xatoi(&ptr, &p1) || !xatoi(&ptr, &p2) || !xatoi(&ptr, &p3)) break;
                    Finfo.fdate = (WORD)(((p1 - 1980) << 9) | ((p2 & 15) << 5) | (p3 & 31));
                    if (!xatoi(&ptr, &p1) || !xatoi(&ptr, &p2) || !xatoi(&ptr, &p3)) break;
                    Finfo.ftime = (WORD)(((p1 & 31) << 11) | ((p2 & 63) << 5) | ((p3 >> 1) & 31));
                    while (_USE_LFN && *ptr == ' ') ptr++;
                    put_rc(f_utime(ptr, &Finfo));
                    break;
    #endif
    #if _USE_LABEL
                case 'b' :  /* fb <name> - Set volume label */
                    while (*ptr == ' ') ptr++;
                    put_rc(f_setlabel(ptr));
                    break;
    #endif
    #if _USE_MKFS
                case 'm' :  /* fm <ld#> <partition rule> <cluster size> - Create file system */
                    if (!xatoi(&ptr, &p1) || (UINT)p1 > 9 || !xatoi(&ptr, &p2) || !xatoi(&ptr, &p3)) break;
                    printf(_T("The volume will be formatted. Are you sure? (Y/n)="));
                    get_uni(ptr, 256);
                    if (*ptr != 'Y') break;
                    sprintf(ptr, _T("%u:"), (UINT)p1);
                    put_rc(f_mkfs(ptr, (BYTE)p2, p3, Buff, sizeof Buff));
                    break;
    #if _MULTI_PARTITION
                case 'p' :  /* fp <pd#> <size1> <size2> <size3> <size4> - Create partition table */
                    {
                        DWORD pts[4];

                        if (!xatoi(&ptr, &p1)) break;
                        xatoi(&ptr, &pts[0]);
                        xatoi(&ptr, &pts[1]);
                        xatoi(&ptr, &pts[2]);
                        xatoi(&ptr, &pts[3]);
                        printf(_T("The physical drive %lu will be re-partitioned. Are you sure? (Y/n)="), p1);
                        fgets(ptr, 256, stdin);
                        if (*ptr != 'Y') break;
                        put_rc(f_fdisk(p1, pts, Buff));
                    }
                    break;
    #endif  /* _MULTI_PARTITION */
    #endif  /* _USE_MKFS */
    #endif  /* !_FS_READONLY */
                default:
                    printf(_T("Not implemented\n"));
                    break;
                }
                break;

            }
        }

	return 0;
}
