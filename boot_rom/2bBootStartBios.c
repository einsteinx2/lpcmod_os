/*

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************

    2003-04-27 hamtitampti
    
 */

#include "2bload.h"
#include "sha1.h"
#include "memory_layout.h"
#include "lpcmod_v1.h"

//Only for debug
enum {
    I2C_LED_RED0 = 0x80,
    I2C_LED_RED1 = 0x40,
    I2C_LED_RED2 = 0x20,
    I2C_LED_RED3 = 0x10,
    I2C_LED_GREEN0 = 0x08,
    I2C_LED_GREEN1 = 0x04,
    I2C_LED_GREEN2 = 0x02,
    I2C_LED_GREEN3 = 0x01
};
extern int decompress_kernel(char*out, char *data, int len);

/* -------------------------  Main Entry for after the ASM sequences ------------------------ */
    // do not change this, this is linked to many many scipts

#define CROMWELL_Memory_pos 	 	CODE_LOC_START
#define PROGRAMM_Memory_2bl 	 	0x00100000
#define CROMWELL_compress_temploc 	0x02000000
#define compressed_image_start 0x3018 // invariable now. Start offset is just after 20 bytes SHA1 + binsize(unsigned int)

#define SHA1Length 20

extern void BootStartBiosLoader ( void )
{
    const unsigned short sysconreg = SYSCON_REG;
    const unsigned short xodusreg= XODUS_CONTROL;
    const unsigned short xblastreg = XBLAST_CONTROL;
    unsigned int cromwellidentify = 1;

    unsigned char sysID, bankReg;
            
    struct SHA1Context context;
    unsigned char SHA1_result[SHA1Length];
    
    unsigned char bootloaderChecksum[SHA1Length];    
    unsigned int bootloadersize;

    unsigned char compressedKernelChecksum[SHA1Length];
    unsigned int loadretry;
    unsigned int compressed_image_size;

    extern int _size_code_2bl;
    const unsigned int boot_ver = BootloaderVersion1;

#if 0
    // Use this to find bugs in boot sequence of 2bl.
    BootPerformPicChallengeResponseAction();

    const unsigned char ledState =
            I2C_LED_GREEN0 | I2C_LED_GREEN1 | I2C_LED_GREEN2 | I2C_LED_GREEN3 |
            I2C_LED_RED0 | I2C_LED_RED1 | I2C_LED_RED2 | I2C_LED_RED3;

    I2CTransmitWord( 0x10, 0x800 | ledState);  // sequencing thanks to Jarin the Penguin!
    I2CTransmitWord( 0x10, 0x701);
    while(1);
#else
    BootPerformPicChallengeResponseAction();    // Must be done very quick. 1.0 boards have SMC with very strict timing.
#endif


    // Get data put there by imagebld
    memcpy(&bootloaderChecksum[0], (void*)PROGRAMM_Memory_2bl, SHA1Length);
    memcpy(&bootloadersize, (void*)(PROGRAMM_Memory_2bl + SHA1Length), sizeof(unsigned int));

    // Check reported size against actual size of data in RAM (excluding bss)
    if(bootloadersize != (int)&_size_code_2bl - PROGRAMM_Memory_2bl)
    {
        goto kill;
    }
    
    // Check SHA1 of 2bl code in RAM.
    SHA1Reset(&context);
    SHA1Input(&context, (void*)(PROGRAMM_Memory_2bl + SHA1Length + sizeof(unsigned int)), bootloadersize - SHA1Length - sizeof(unsigned int));
    SHA1Result(&context, SHA1_result);
    
    // Avoid further 2bl execution if mismatch. Avoid potential freeze.
    if(memcmp(&bootloaderChecksum[0], &SHA1_result[0], SHA1Length))
    {
        // Bad, the checksum does not match, but we can nothing do now. Hope we'll jump successfully to "kill"!
        goto kill;
    }
    // HEHE, the Image we copy'd into ram is SHA-1 hash identical, this is Optimum
    // Sets the Graphics Card to 60 MB start address
    (*(unsigned int*)0xFD600800) = FB_START;
    
    // Lets go, we have finished, the Most important Startup, we have now a valid Micro-loder im Ram
    // we are quite happy now
    
    for (loadretry = 0; loadretry < 10; loadretry++)
    {
        // Copy From Flash To RAM
        // Copy Kernel SHA-1 checksum
        memcpy(&compressedKernelChecksum[0], (void*)(LPCFlashadress + 0x3000), SHA1Length); // Kernel data always starts at offset 0x3000 in flash.
        memcpy(&compressed_image_size, (void*)(LPCFlashadress + 0x3000 + SHA1Length), sizeof(unsigned int));

        // Arbitrary size validation
        if(compressed_image_size < 50000 || compressed_image_size > (256 * 1024 - compressed_image_start - (4 * 1024)))
        {
            goto kill;
        }

        // Copy GZipped Kernel
        memcpy((void*)CROMWELL_compress_temploc, (void*)(LPCFlashadress + compressed_image_start), compressed_image_size);
        
        // Lets Look, if we have got a Valid thing from Flash            
        SHA1Reset(&context);
        SHA1Input(&context, (void*)(CROMWELL_compress_temploc), compressed_image_size);
        SHA1Result(&context, SHA1_result);
        


        if(memcmp(&compressedKernelChecksum[0], SHA1_result, SHA1Length) == 0)
        {
            // The Checksum is good                          
            // We start the Cromwell immediatly
            BufferIN = (unsigned char *)(CROMWELL_compress_temploc);
            BufferINlen = compressed_image_size;
            BufferOUT = (unsigned char *)CROMWELL_Memory_pos;
            memset((void *)CROMWELL_Memory_pos, 0x00, 0x400000);
            decompress_kernel(BufferOUT, BufferIN, BufferINlen);
            
            // This is a config bit in Cromwell, telling the Cromwell, that it is a Cromwell and not a Xromwell
            // Will be cromwell_config in Cromwell
            memcpy((void*)(CROMWELL_Memory_pos + 0x20), &cromwellidentify, sizeof(cromwellidentify));
            // Will be cromwell_retryload in Cromwell
            memcpy((void*)(CROMWELL_Memory_pos + 0x20 + sizeof(cromwellidentify)), &loadretry, sizeof(loadretry));
            // Will be cromwell_2blversion in Cromwell
            memcpy((void*)(CROMWELL_Memory_pos + 0x20 + sizeof(cromwellidentify) + sizeof(loadretry) ), &boot_ver, sizeof(boot_ver));
            // Will be cromwell_2blsize in Cromwell
            memcpy((void*)(CROMWELL_Memory_pos + 0x20 + sizeof(cromwellidentify) + sizeof(loadretry) + sizeof(boot_ver)), &bootloadersize, sizeof(bootloadersize));

            // We now jump to the cromwell, Good bye 2bl loader
            // This means: jmp CROMWELL_Memory_pos == 0x03800000
            __asm __volatile__ (
            "cld\n"
            "ljmp $0x10, $0x03800000\n"
            );
            // We are not Longer here
            break;
        }
    }
    
    // Bad, we did not get a valid image to RAM
kill:

    // Compatible modchip?
    __asm__ __volatile__ ("inb %w1,%0":"=a" (sysID):"Nd" (sysconreg));

    if(sysID == SYSCON_ID_V1 || sysID == SYSCON_ID_V1_PRE_EDITION || sysID == SYSCON_ID_XT)
    {
        // On OS bank? (or close enough)
        __asm__ __volatile__ ("inb %w1,%0":"=a" (bankReg):"Nd" (xodusreg));
        if(bankReg == 0x0C)
        {
            // Boot 512KB bank
            __asm__ ("out %%al, %%dx" : : "a" (BNK512), "d" (xblastreg));
            I2CTransmitWord(0x10, 0x0201);
            while(1);
        }
    }

    // Power_cycle
    I2CTransmitWord(0x10, 0x0240);
    while(1);
}
