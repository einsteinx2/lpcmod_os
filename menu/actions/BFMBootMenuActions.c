/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "BFMBootMenuActions.h"
#include "boot.h"
#include "BootIde.h"
#include "BootFATX.h"
#include "video.h"
#include "lpcmod_v1.h"
#include "memory_layout.h"

int evoxrom_detect(void *rom, unsigned long rom_size);
void *evoxrom_prepare(void *rom, unsigned long rom_size);
int metoobfm_detect(void *rom, unsigned long rom_size);
void *metoobfm_prepare(void *rom, unsigned long rom_size);
char *strh_dnzcpy(char *d, const char *s, size_t n);

struct metoobfm_footer {
    u32 reserved;
    u16 loader_ofs;
    u16 kernel_param_size;
    u16 kernel_param_ofs;
    u16 size_2bl;
    u32 base_2bl;
    u32 magic;          /* BFM1 */
};

void bootBFMBios(void *fname){
    int res;
    u8 * fileBuf;
    FATXFILEINFO fileinfo;
    FATXPartition *partition;

    partition = OpenFATXPartition (0, SECTOR_SYSTEM, SYSTEM_SIZE);
    fileBuf = (u8 *) malloc (1024 * 1024);  //1MB buffer(max BIOS size)
    memset (fileBuf, 0x00, 1024 * 1024);   //Fill with 0.
    
    //res = LoadFATXFilefixed(partition, fname, &fileinfo, (char*)0x100000);
    res = LoadFATXFile(partition, fname, &fileinfo);
    CloseFATXPartition (partition);
    if (!res) {
        printk ("\n\n\n\n\n           Loading BIOS failed");
        dots ();
        cromwellError ();
        while (1)
            ;
    }
    memcpy(fileBuf, fileinfo.buffer, fileinfo.fileSize);
    free(fileinfo.buffer);
    fileinfo.buffer = fileBuf;
    decodeAndSetupBFMBios(fileinfo.buffer, fileinfo.fileSize);
    free(fileinfo.buffer);

    return;
}

void decodeAndSetupBFMBios(unsigned char *fileBuf, unsigned int fileSize){
    u32 PhysicalRomPos = 0x02f01000;//(u32 *)KERNEL_PM_CODE;   //Position in RAM
    u32 EntryPoint2BL;

    switch(fileSize){
        case 262144:
            memcpy(&fileBuf[262144], fileBuf, 262144);
            fileSize += fileSize;
            //Fall through
        case 524288:
            memcpy(&fileBuf[524288], fileBuf, 524288);
            fileSize += fileSize;
            break;
        default:
            break;
    }
    if(fileSize != 1048576){
        printk("\n              Wrong fileSize = %u", fileSize);
        while ((risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_A) != 1)) wait_ms(10);
        return;
    }

    memcpy((void*)PhysicalRomPos, fileBuf, fileSize);
    if (metoobfm_detect((void *)fileBuf, SHADOW_ROM_SIZE)) {
        //dprintf("Metoo BFM 2bl footer detected\n");
        EntryPoint2BL = (u32)metoobfm_prepare((void *)fileBuf, SHADOW_ROM_SIZE);
        printk("\n              metoo BFM detected.");
    } else if (evoxrom_detect((void *)fileBuf, SHADOW_ROM_SIZE)) {
        //dprintf("EvoX M8 2bl type detected\n");
        EntryPoint2BL = (u32)evoxrom_prepare((void *)fileBuf, SHADOW_ROM_SIZE);
        printk("\n              EvoxM8 BFM detected.");
    } else {
        //dprintf("Assuming standard 2bl format\n");
        //EntryPoint2BL = std2bl_prepare((void *)fileBuf, SHADOW_ROM_SIZE, &entry);
        printk("\n              Other BFM detected. Not good.");
    }

    printk("\n              EntryPoint2BL addr = 0x%08X.    0 contains 0x%02X vs 0xCC", EntryPoint2BL, *(u8*)EntryPoint2BL);
    printk("\n              PhysicalRomPos addr = 0x%08X.    0 contains 0x%02X vs 0x09", PhysicalRomPos, *(u8*)PhysicalRomPos);
    printk("\n\n           Press Button 'A' to continue.");
    while ((risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_A) != 1)) wait_ms(10);
    I2CTransmitByteGetReturn(0x54, 0x58); /* "Fuck me gently with a chainsaw"*/
    BootVideoClearScreen(&jpegBackdrop, 0, 0xffff);

    __asm__ __volatile__(
        "push   %%eax\n"
        "push   %%eax\n"
        "push   %%eax\n"
        "push   %%eax\n"
        "sgdt   0x2(%%esp)\n"
        "pop    %%eax\n"
        "pop    %%eax\n"
        "mov    %%cs,%%edx\n"
        "add    %%edx,%%eax\n"
        "mov    %%cs,0x4(%%esp)\n"
        "cli\n"
        "movw   $0xffff,(%%eax)\n"      /* Code segment size enlargement is */
        "orb    $0xb,0x6(%%eax)\n"      /* required for 5530+ kernels */
        "ljmp   *(%%esp)\n"
        : /* no output */
        : "a" (EntryPoint2BL), "c" (PhysicalRomPos)
        );

    printk("\n              ASM seq skipped. How?");
    while(1);
    return;
}

int evoxrom_detect(void *rom, unsigned long rom_size){
    if (strncmp(rom + rom_size - 0x3000 + 0x2e58, "$EvoxRom$", 10) != 0)
        return false;

    return true;
}

void *evoxrom_prepare(void *rom, unsigned long rom_size){
    void *virt2bl;
    void *copyptr;

    //dprintf("Allocate 2bl mem\n");

    virt2bl = (u32 *)MIN_2BL;

    /* Copy the 2bl to the appropriate location */

    copyptr = (void *)(rom + (rom_size - 0x3000));
    //dprintf("Copying 2bl\n");
    memcpy(virt2bl, copyptr, 0x3000);

    //dprintf("Calculating 2bl entry point\n");

    return (void *)(*(u32 *)virt2bl - 0x90000 + 0x80400000);
}

int metoobfm_detect(void *rom, unsigned long rom_size){
    if (*(u32 *)(rom + rom_size - 4) == METOOBFM_MAGIC1)
        return true;

    return false;
}

void *metoobfm_prepare(void *rom, unsigned long rom_size){
    struct metoobfm_footer *ft;
    void *base;
    void *copyptr;
    char *param;

    ft = (struct metoobfm_footer *)(rom + rom_size - sizeof(struct metoobfm_footer));

    //dprintf("Allocate 2bl mem\n");

    base = (u32 *)ft->base_2bl;

    /* Copy the 2bl to the appropriate location */

    copyptr = (void *)(rom + rom_size - ft->size_2bl);
    //dprintf("Copying 2bl\n");
    memcpy(base, copyptr, ft->size_2bl);

    if (ft->kernel_param_size != 0) {
        //dprintf("Patching kernel param string\n");
        param = (char *)(ft->kernel_param_ofs + base);
        strh_dnzcpy(param, " /SHADOW /HDBOOT", ft->kernel_param_size);

        //if (entry->screen == CFG_SCREEN_KEEP)
        //    video_add_avsave_param(param, ft->kernel_param_size);

        param[ft->kernel_param_size - 1] = 0;

        //dprintf("Parameter string: \"%s\"\n", param);
    }

    //dprintf("Calculating 2bl entry point\n");

    return (void *)(ft->loader_ofs + base + 0x80000000);
}

//Probably replaceable by snprintf
char *strh_dnzcpy(char *d, const char *s, size_t n)
{
    char *dest = d;

    if (n == 0)
        return NULL;

    while (n > 0 && *s) {
        n--; *d++ = *s++;
    }

    if (n > 0)
        *d = '\0';
    else
        *(d-1) = '\0';

    return dest;
}
