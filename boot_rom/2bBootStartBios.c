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

extern int decompress_kernel(char*out, char *data, int len);

/* -------------------------  Main Entry for after the ASM sequences ------------------------ */
    // do not change this, this is linked to many many scipts

#define CROMWELL_Memory_pos 	 	CODE_LOC_START
#define PROGRAMM_Memory_2bl 	 	0x00100000
#define CROMWELL_compress_temploc 	0x02000000

#define SHA1Length 20

extern void BootStartBiosLoader ( void )
{
    unsigned int cromwellidentify = 1;
    unsigned int flashbank = 1;  // Default Bank
            
    struct SHA1Context context;
    unsigned char SHA1_result[SHA1Length];
        
    unsigned char bootloaderChecksum[SHA1Length];
    unsigned int bootloadersize;
    unsigned int loadretry;
    unsigned int compressed_image_start;
    unsigned int compressed_image_size;
    unsigned int Biossize_type;

//Use this to find bugs in boot sequence of 2bl.
#if 0
    BootPerformPicChallengeResponseAction();
    I2cSetFrontpanelLed(
    		I2C_LED_GREEN0 | I2C_LED_GREEN1 | I2C_LED_GREEN2 | I2C_LED_GREEN3 |
			I2C_LED_RED0 | I2C_LED_RED1 | I2C_LED_RED2 | I2C_LED_RED3
    );
    while(1);
#endif
    
    BootPerformPicChallengeResponseAction();    // MNust be done very quick. 1.0 boards have SMC with very strict timing.

    memcpy(&bootloaderChecksum[0], (void*)PROGRAMM_Memory_2bl, SHA1Length);
    memcpy(&bootloadersize, (void*)(PROGRAMM_Memory_2bl + SHA1Length), sizeof(unsigned int));
    memcpy(&compressed_image_start, (void*)(PROGRAMM_Memory_2bl + SHA1Length + sizeof(bootloadersize)), sizeof(unsigned int));
    memcpy(&compressed_image_size, (void*)(PROGRAMM_Memory_2bl + SHA1Length + sizeof(bootloadersize) + sizeof(compressed_image_start)), sizeof(unsigned int));
    memcpy(&Biossize_type, (void*)(PROGRAMM_Memory_2bl + SHA1Length + sizeof(bootloadersize) + sizeof(compressed_image_start) + sizeof(compressed_image_size)), sizeof(unsigned int));   //0 for 256KB image. 1 for 1MB image.


    SHA1Reset(&context);
    SHA1Input(&context, (void*)(PROGRAMM_Memory_2bl + SHA1Length), bootloadersize - SHA1Length);
    SHA1Result(&context, SHA1_result);
            
    if (memcmp(&bootloaderChecksum[0], &SHA1_result[0], 20 * sizeof(char)))
    {
        // Bad, the checksum does not match, but we can nothing do now. Reset console.
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
        //Copy Kernel SHA-1 checksum
        memcpy(&bootloaderChecksum[0], (void*)(0xff000000 + compressed_image_start), SHA1Length);

        // Copy GZipped Kernel
        memcpy((void*)CROMWELL_compress_temploc, (void*)(0xff000000+compressed_image_start + SHA1Length),compressed_image_size);

        // Set remaining space after compressed data to 0x0.
        //XXX: Is this necessary?
        memset((void*)(CROMWELL_compress_temploc + compressed_image_size), 0x00, 20*1024);
        
        // Lets Look, if we have got a Valid thing from Flash            
        SHA1Reset(&context);
        SHA1Input(&context, (void*)(CROMWELL_compress_temploc), compressed_image_size);
        SHA1Result(&context, SHA1_result);
        

        if(memcmp(&bootloaderChecksum[0], SHA1_result, SHA1Length) == 0)
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
            // Will be cromwell_loadbank in Cromwell
            memcpy((void*)(CROMWELL_Memory_pos + 0x20 + sizeof(cromwellidentify) + sizeof(loadretry)), &flashbank, sizeof(flashbank));
            // Will be cromwell_Biostype in Cromwell
            memcpy((void*)(CROMWELL_Memory_pos + 0x20 + sizeof(cromwellidentify) + sizeof(loadretry) + sizeof(flashbank)), &Biossize_type, sizeof(Biossize_type));


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
    
    // Bad, we did not get a valid image to RAM, we stop and display a error
kill:
    // Power_cycle
    I2CTransmitWord(0x10, 0x0240);
    I2CTransmitWord(0x10, 0x0240);

    while(1);
}
