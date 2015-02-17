/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "boot.h"
#include "BootFlash.h"
#include <stdio.h>
#include "memory_layout.h"
#include "lpcmod_v1.h"

int sprintf(char * buf, const char *fmt, ...);

// gets device ID, sets pof up accordingly
// returns true if device okay or false for unrecognized device

bool BootFlashGetDescriptor( OBJECT_FLASH *pof, KNOWN_FLASH_TYPE * pkft )
{
    bool fSeen=false;
    u8 baNormalModeFirstTwoBytes[2];
    int nPos=0;

    pof->m_fIsBelievedCapableOfWriteAndErase=true;
    pof->m_szAdditionalErrorInfo[0]='\0';

    baNormalModeFirstTwoBytes[0]=pof->m_pbMemoryMappedStartAddress[0];
    baNormalModeFirstTwoBytes[1]=pof->m_pbMemoryMappedStartAddress[1];

    // make sure the flash state machine is reset

    pof->m_pbMemoryMappedStartAddress[0x5555]=0xf0;
    pof->m_pbMemoryMappedStartAddress[0x5555]=0xaa;
    pof->m_pbMemoryMappedStartAddress[0x2aaa]=0x55;
    pof->m_pbMemoryMappedStartAddress[0x5555]=0xf0;

    // read flash ID

    pof->m_pbMemoryMappedStartAddress[0x5555]=0xaa;
    pof->m_pbMemoryMappedStartAddress[0x2aaa]=0x55;
    pof->m_pbMemoryMappedStartAddress[0x5555]=0x90;

    pof->m_bManufacturerId=pof->m_pbMemoryMappedStartAddress[0];
    pof->m_bDeviceId=pof->m_pbMemoryMappedStartAddress[1];

    pof->m_pbMemoryMappedStartAddress[0x5555]=0xf0;
    // interpret device ID info


    bool fMore=true;
    while(fMore) {
        if(!pkft->m_bManufacturerId) {
            fMore=false; continue;
        }
        if((pkft->m_bManufacturerId == pof->m_bManufacturerId) &&
            (pkft->m_bDeviceId == pof->m_bDeviceId)) {
            fSeen=true;
            fMore=false;
            //Initially printd spaces before actual string. I don't want this...
            //nPos+=sprintf(&pof->m_szFlashDescription[nPos], "           %s (%dK)", pkft->m_szFlashDescription, pkft->m_dwLengthInBytes/1024);
            nPos+=sprintf(&pof->m_szFlashDescription[nPos], "%s", pkft->m_szFlashDescription);
            pof->m_dwLengthInBytes = pkft->m_dwLengthInBytes;
        }
        pkft++;
    }



    if(!fSeen) {
        if(
            (baNormalModeFirstTwoBytes[0]==pof->m_bManufacturerId) &&
            (baNormalModeFirstTwoBytes[1]==pof->m_pbMemoryMappedStartAddress[1])) { // we didn't get anything worth reporting
            sprintf(pof->m_szFlashDescription, "           Read Only??? manf=0x%02X, dev=0x%02X", pof->m_bManufacturerId, pof->m_bDeviceId);
        }
        else { // we got what is probably an unknown flash type
            sprintf(pof->m_szFlashDescription, "           manf=0x%02X, dev=0x%02X", pof->m_bManufacturerId, pof->m_bDeviceId);
        }
    }

    return fSeen;
}

 // uses the block erase function on the flash to erase the minimal footprint
 // needed to cover pof->m_dwStartOffset .. (pof->m_dwStartOffset+pof->m_dwLengthUsedArea)

 #define MAX_ERASE_RETRIES_IN_4KBLOCK_BEFORE_FAILING 4
 
bool BootFlashEraseMinimalRegion( OBJECT_FLASH *pof)
{
    u32 dw=pof->m_dwStartOffset;
    u32 dwLen=pof->m_dwLengthUsedArea;
    u32 dwLastEraseAddress=0xffffffff;
    int nCountEraseRetryIn4KBlock=MAX_ERASE_RETRIES_IN_4KBLOCK_BEFORE_FAILING;

    pof->m_szAdditionalErrorInfo[0]='\0';
    
    if(pof->m_pcallbackFlash!=NULL)
        if(!(pof->m_pcallbackFlash)(pof, EE_ERASE_START, 0, 0)) {
            strcpy(pof->m_szAdditionalErrorInfo, "           Erase Aborted");
            return false;
        }
    while(dwLen) {

        if(pof->m_pbMemoryMappedStartAddress[dw]!=0xff) { // something needs erasing

            u8 b;

            if((dwLastEraseAddress & 0xfffff000)==(dw & 0xfffff000)) { // same 4K block?
                nCountEraseRetryIn4KBlock--;
                if(nCountEraseRetryIn4KBlock==0) { // run out of tries in this 4K block
                    if(pof->m_pcallbackFlash!=NULL) {
                        (pof->m_pcallbackFlash)(pof, EE_ERASE_ERROR, dw-pof->m_dwStartOffset, pof->m_pbMemoryMappedStartAddress[dw]);
                        (pof->m_pcallbackFlash)(pof, EE_ERASE_END, 0, 0);
                    }
                    sprintf(pof->m_szAdditionalErrorInfo, "           Erase failed for block at +0x%x, reads as 0x%02X", dw, pof->m_pbMemoryMappedStartAddress[dw]);
                    return false; // failure
                }
            }
            else {
                nCountEraseRetryIn4KBlock=MAX_ERASE_RETRIES_IN_4KBLOCK_BEFORE_FAILING;  // different block, reset retry counter
                dwLastEraseAddress=dw;
            }

            u32 dwCountTries=0;

            pof->m_pbMemoryMappedStartAddress[0x5555]=0xaa;
            pof->m_pbMemoryMappedStartAddress[0x2aaa]=0x55;
            pof->m_pbMemoryMappedStartAddress[0x5555]=0x80;

            pof->m_pbMemoryMappedStartAddress[0x5555]=0xaa;
            pof->m_pbMemoryMappedStartAddress[0x2aaa]=0x55;
            pof->m_pbMemoryMappedStartAddress[dw]=0x50; // erase the block containing the non 0xff guy

            b=pof->m_pbMemoryMappedStartAddress[dw];  // waits until b6 is no longer toggling on each read
            while((pof->m_pbMemoryMappedStartAddress[dw]&0x40)!=(b&0x40)) {
                dwCountTries++; b^=0x40;
            }

            if(dwCountTries<3) { // <3 means never entered busy mode - block erase code 0x50 not supported, try alternate
                pof->m_pbMemoryMappedStartAddress[0x5555]=0xaa;
                pof->m_pbMemoryMappedStartAddress[0x2aaa]=0x55;
                pof->m_pbMemoryMappedStartAddress[0x5555]=0xf0;

                pof->m_pbMemoryMappedStartAddress[0x5555]=0xaa;
                pof->m_pbMemoryMappedStartAddress[0x2aaa]=0x55;
                pof->m_pbMemoryMappedStartAddress[0x5555]=0x80;

                pof->m_pbMemoryMappedStartAddress[0x5555]=0xaa;
                pof->m_pbMemoryMappedStartAddress[0x2aaa]=0x55;
                pof->m_pbMemoryMappedStartAddress[dw]=0x30; // erase the sector containing the non 0xff guy

                b=pof->m_pbMemoryMappedStartAddress[dw];
                dwCountTries=0;
                while((pof->m_pbMemoryMappedStartAddress[dw]&0x40)!=(b&0x40)) {
                    dwCountTries++; b^=0x40;
                }
            }

            // if we had a couple of unsuccessful tries at block erase already, try chip erase
            // safety features...
            if((dwCountTries<3) &&  // other commands did not work at all
               (nCountEraseRetryIn4KBlock<2) &&  // had multiple attempts at them already
               (pof->m_dwStartOffset==0) && // reprogramming whole chip .. starting from start
               (pof->m_dwLengthUsedArea == pof->m_dwLengthInBytes)  // and doing the whole length of the chip
                ) { // <3 means never entered busy mode - block erase code 0x30 not supported
                #if 1
                printk("           Trying to erase whole chip\n");
                #endif
                pof->m_pbMemoryMappedStartAddress[0x5555]=0xaa;
                pof->m_pbMemoryMappedStartAddress[0x2aaa]=0x55;
                pof->m_pbMemoryMappedStartAddress[0x5555]=0xf0;

                pof->m_pbMemoryMappedStartAddress[0x5555]=0xaa;
                pof->m_pbMemoryMappedStartAddress[0x2aaa]=0x55;
                pof->m_pbMemoryMappedStartAddress[0x5555]=0x80;

                pof->m_pbMemoryMappedStartAddress[0x5555]=0xaa;
                pof->m_pbMemoryMappedStartAddress[0x2aaa]=0x55;
                pof->m_pbMemoryMappedStartAddress[0x5555]=0x10; // chip erase ONLY available on W49F020

                b=pof->m_pbMemoryMappedStartAddress[dw];
                dwCountTries=0;
                while((pof->m_pbMemoryMappedStartAddress[dw]&0x40)!=(b&0x40)) {
                    dwCountTries++; b^=0x40;
                }
           }
           continue; // retry reading this address without moving on
        }

            // update callback every 1K addresses
        if((dw&0x3ff)==0) {
            if(pof->m_pcallbackFlash!=NULL) {
                if(!(pof->m_pcallbackFlash)(pof, EE_ERASE_UPDATE, dw-pof->m_dwStartOffset, pof->m_dwLengthUsedArea)) {
                    strcpy(pof->m_szAdditionalErrorInfo, "           Erase Aborted");
                    return false;
                }
            }
        }

        dwLen--; dw++;
    }

    if(pof->m_pcallbackFlash!=NULL)
        if(!(pof->m_pcallbackFlash)(pof, EE_ERASE_END, 0, 0))
           return false;
    
    return true;
}

bool BootFlashErase4KSector( OBJECT_FLASH *pof)
{
    u32 dw=pof->m_dwStartOffset;
    u32 dwLen=0x1000;                   //4KB length
    u32 dwLastEraseAddress=0xffffffff;

    pof->m_szAdditionalErrorInfo[0]='\0';

    if(pof->m_pcallbackFlash!=NULL)
        if(!(pof->m_pcallbackFlash)(pof, EE_ERASE_START, 0, 0)) {
            strcpy(pof->m_szAdditionalErrorInfo, "           Erase Aborted");
            return false;
        }
    while(dwLen) {

        if(pof->m_pbMemoryMappedStartAddress[dw]!=0xff) { // something needs erasing

            u8 b;

            u32 dwCountTries=0;

            pof->m_pbMemoryMappedStartAddress[0x5555]=0xaa;
            pof->m_pbMemoryMappedStartAddress[0x2aaa]=0x55;
            pof->m_pbMemoryMappedStartAddress[0x5555]=0x80;

            pof->m_pbMemoryMappedStartAddress[0x5555]=0xaa;
            pof->m_pbMemoryMappedStartAddress[0x2aaa]=0x55;
            pof->m_pbMemoryMappedStartAddress[dw]=0x30; // erase the block containing the non 0xff guy

            b=pof->m_pbMemoryMappedStartAddress[dw];  // waits until b6 is no longer toggling on each read
            while((pof->m_pbMemoryMappedStartAddress[dw]&0x40)!=(b&0x40)) {
                dwCountTries++; b^=0x40;
            }

            if(dwCountTries<3) { // <3 means never entered busy mode - block erase code 0x50 not supported.
                strcpy(pof->m_szAdditionalErrorInfo, "           Sector-Erase not supported.");
                return false;
            }
           continue; // retry reading this address without moving on
        }

            // update callback every 1K addresses
        if((dw&0x3ff)==0) {
            if(pof->m_pcallbackFlash!=NULL) {
                if(!(pof->m_pcallbackFlash)(pof, EE_ERASE_UPDATE, dw-pof->m_dwStartOffset, pof->m_dwLengthUsedArea)) {
                    strcpy(pof->m_szAdditionalErrorInfo, "           Erase Aborted");
                    return false;
                }
            }
        }

        dwLen--; dw++;
    }

    if(pof->m_pcallbackFlash!=NULL)
        if(!(pof->m_pcallbackFlash)(pof, EE_ERASE_END, 0, 0))
           return false;

    return true;
}

    // program the flash from the data in pba
    // length of valid data in pba held in pof->m_dwLengthUsedArea

bool BootFlashProgram( OBJECT_FLASH *pof, u8 *pba)
{
    u32 dw=pof->m_dwStartOffset;
    u32 dwLen=pof->m_dwLengthUsedArea;
    u32 dwSrc=0;
    u32 dwLastProgramAddress=0xffffffff;
    int nCountProgramRetries=4;

    pof->m_szAdditionalErrorInfo[0]='\0';
    if(pof->m_pcallbackFlash!=NULL)
        if(!(pof->m_pcallbackFlash)(pof, EE_PROGRAM_START, 0, 0)) {
            strcpy(pof->m_szAdditionalErrorInfo, "           Program Aborted");
            return false;
    }

        // program

    while(dwLen) {

        if(pof->m_pbMemoryMappedStartAddress[dw]!=pba[dwSrc]) { // needs programming

            if(dwLastProgramAddress==dw) {
                nCountProgramRetries--;
                if(nCountProgramRetries==0) {
                    if(pof->m_pcallbackFlash!=NULL) {
                        (pof->m_pcallbackFlash)(pof, EE_PROGRAM_ERROR, dw, (((u32)pba[dwSrc])<<8) |pof->m_pbMemoryMappedStartAddress[dw] );
                        (pof->m_pcallbackFlash)(pof, EE_PROGRAM_END, 0, 0);
                    }
                    sprintf(pof->m_szAdditionalErrorInfo, "           Program failed for byte at +0x%x; wrote 0x%02X, read 0x%02X", dw, pba[dwSrc], pof->m_pbMemoryMappedStartAddress[dw]);
                    return false;
                }
            }
            else {
                nCountProgramRetries=4;
                dwLastProgramAddress=dw;
            }


            u8 b;
            pof->m_pbMemoryMappedStartAddress[0x5555]=0xaa;
            pof->m_pbMemoryMappedStartAddress[0x2aaa]=0x55;
            pof->m_pbMemoryMappedStartAddress[0x5555]=0xa0;
            pof->m_pbMemoryMappedStartAddress[dw]=pba[dwSrc]; // perform programming action
            b=pof->m_pbMemoryMappedStartAddress[dw];  // waits until b6 is no longer toggling on each read
            while((pof->m_pbMemoryMappedStartAddress[dw]&0x40)!=(b&0x40))
                b^=0x40;

            continue;  // does NOT advance yet
        }

        if((dw&0x3ff)==0){
            if(pof->m_pcallbackFlash!=NULL)
                if(!(pof->m_pcallbackFlash)(pof, EE_PROGRAM_UPDATE, dwSrc, pof->m_dwLengthUsedArea)) {
                    strcpy(pof->m_szAdditionalErrorInfo, "           Program Aborted");
                    return false;
                }
        }
        dwLen--; dw++; dwSrc++;
    }

    if(pof->m_pcallbackFlash!=NULL){
        if(!(pof->m_pcallbackFlash)(pof, EE_PROGRAM_END, 0, 0))
            return false;

        // verify
        if(!(pof->m_pcallbackFlash)(pof, EE_VERIFY_START, 0, 0))
            return false;
    }
    dw=pof->m_dwStartOffset;
    dwLen=pof->m_dwLengthUsedArea;
    dwSrc=0;

    while(dwLen) {

        if(pof->m_pbMemoryMappedStartAddress[dw]!=pba[dwSrc]) { // verify error
            if(pof->m_pcallbackFlash!=NULL){
                if(!(pof->m_pcallbackFlash)(pof, EE_VERIFY_ERROR, dw, (((u32)pba[dwSrc])<<8) |pof->m_pbMemoryMappedStartAddress[dw]))
                    return false;

                if(!(pof->m_pcallbackFlash)(pof, EE_VERIFY_END, 0, 0))
                    return false;

/*                    if(!(pof->m_pcallbackFlash)(pof, EE_VERIFY_UPDATE, dwSrc, pof->m_dwLengthUsedArea)) {
                        strcpy(pof->m_szAdditionalErrorInfo, "           Program Aborted");
                        return false;
                    }
*/
            }
            return false;
        }
        else {
            if(pof->m_pcallbackFlash!=NULL){
            	if((dwLen % 1024) ==0){
                    if(!(pof->m_pcallbackFlash)(pof, EE_VERIFY_UPDATE, dwSrc, pof->m_dwLengthUsedArea)) {
                           strcpy(pof->m_szAdditionalErrorInfo, "           Program Aborted");
                            return false;
                    }
                }
            }
        }

        dwLen--; dw++; dwSrc++;
    }

        if(pof->m_pcallbackFlash!=NULL)
            if(!(pof->m_pcallbackFlash)(pof, EE_VERIFY_END, 0, 0))
                return false;

    return true;
}


//Not needed right now.
/*
u8 GetByteFromFlash(int myaddress) {
  OBJECT_FLASH of;
  of.m_pbMemoryMappedStartAddress = (u8 *)LPCFlashadress;
  return xGetByteFromFlash(&of, myaddress);
}

u8 xGetByteFromFlash(OBJECT_FLASH *myflash, int myaddress) {
  return myflash->m_pbMemoryMappedStartAddress[myaddress]; }
*/

void WriteToIO(u16 _port, u8 _data)
{
   __asm__ ("out %%al, %%dx" : : "a" (_data), "d" (_port));
}

u8 ReadFromIO(u16 address)
{
   unsigned char data;
   __asm__ __volatile__ ("inb %w1,%0":"=a" (data):"Nd" (address));
   return data;
}

void BootFlashGetOSSettings(_LPCmodSettings *LPCmodSettings) {
    OBJECT_FLASH of;
    int i;
    if(fHasHardware == SYSCON_ID_V1 || fHasHardware == SYSCON_ID_XT || cromwell_config==CROMWELL){
        //memset(&of,0xFF,sizeof(of));
        of.m_pbMemoryMappedStartAddress=(u8 *)LPCFlashadress;               //Only thing we need really.
        for (i = 0; i < sizeof(_LPCmodSettings); i++){        //Length of reserved flash space for persistent setting data.
            *((u8*)LPCmodSettings + i) = of.m_pbMemoryMappedStartAddress[0x3f000 + i];        //Starts at 0x3f000 in flash
        }
    }
}

//Saves persistent settings at 0x3f000 offset on flash.
void BootFlashSaveOSSettings(void) {
    OBJECT_FLASH of;
    u8 * lastBlock;
    u32 blocksize;

    // A bit hacky, but easier to maintain.
    const KNOWN_FLASH_TYPE aknownflashtypesDefault[] = {
        #include "flashtypes.h"
    };
    if(fHasHardware == SYSCON_ID_V1 || fHasHardware == SYSCON_ID_XT || cromwell_config==CROMWELL){
    			
        memset(&of,0xFF,sizeof(of));
        of.m_pbMemoryMappedStartAddress=(u8 *)LPCFlashadress;
 
/*       
        if(fHasHardware == SYSCON_ID_XX1 ||
           fHasHardware == SYSCON_ID_XX2 ||
           fHasHardware == SYSCON_ID_XXOPX ||
           fHasHardware == SYSCON_ID_XX3){
            IoOutputByte(SMARTXX_FLASH_WRITEPROTECT , 1);       //Enable flash write on SmartXX mods.
        }
*/
        if(BootFlashGetDescriptor(&of, (KNOWN_FLASH_TYPE *)&aknownflashtypesDefault[0])) {        //Still got flash to interface?
            if(assert4KBErase(&of)){                //XBlast Lite V1 has 4KB-sector erase capability
                blocksize = 4 * 1024;                       //4KB allocation
            }
            else {                                           //Other devices, we assume 64KB block erasing only
                blocksize = 64 * 1024;
            }

            lastBlock = (u8 *)malloc(blocksize);
    
            if(currentFlashBank != BNKOS)              //Just to be sure, can only be true on a XBlast mod.
                switchOSBank(BNKOS);


            memcpy(lastBlock,(const u8*)((&of)->m_pbMemoryMappedStartAddress) + (0x40000 - blocksize), blocksize);    //Copy content of flash into temp memory allocation.

            if(memcmp(&(lastBlock[blocksize-(4*1024)]),(u8*)&LPCmodSettings,sizeof(_LPCmodSettings))) {            //At least one setting changed from what's currently in flash.
                if(fHasHardware == SYSCON_ID_X3)	//Extra warning for X3 user.
    		    if(ConfirmDialog("           Are you on the same flash bank?", 1))
    		        return;
                memcpy(&(lastBlock[blocksize-(4*1024)]),(const u8*)&LPCmodSettings,sizeof(_LPCmodSettings));    //Copy settings at the start of the 4KB block.
                if(scriptSavingPtr != NULL){    //There's a script to save
                    memcpy(&(lastBlock[blocksize-(4*1024) + sizeof(_LPCmodSettings)]), scriptSavingPtr, LPCmodSettings.firstScript.nextEntryPosition - 1 - sizeof(_LPCmodSettings));
                    free(scriptSavingPtr);
                    scriptSavingPtr = NULL;
                }
                BootFlashSettings(lastBlock,(0x40000 - blocksize),blocksize);            //Even if bank is bigger than 256KB, we only save on first 256KB part.
            }
            free(lastBlock);
        }
/*
        if(fHasHardware == SYSCON_ID_XX1 ||
           fHasHardware == SYSCON_ID_XX2 ||
           fHasHardware == SYSCON_ID_XXOPX ||
           fHasHardware == SYSCON_ID_XX3){
            IoOutputByte(SMARTXX_FLASH_WRITEPROTECT , 0);       //Disable flash write on SmartXX mods.
        }
*/
    }
}

int assertOSUpdateValidInput(u8 * inputFile) {
    int result = 1;    //Start off assuming image file is not XBlast OS.
    int i;
    char * compareString = "XBlast OS";
    
    OBJECT_FLASH of;
    
    memset(&of,0xFF,sizeof(of));
    of.m_pbMemoryMappedStartAddress=(u8 *)LPCFlashadress;
    
    for(i = 0x3FFD0; i < 0x3FFD8; i++) {
        if(compareString[i] != inputFile[i])
            return result;        //Not OS update image.
    }
    //Passed the loop asserts image is valid OS image.
    result = 0;
    return result;
}

bool assert4KBErase(OBJECT_FLASH *pof){
    switch(pof->m_bManufacturerId){
        case 0xbf:                      //SST
            switch(pof->m_bDeviceId){
                case 0x4c:              //49LF devices are good.
                case 0x51:
                case 0x52:
                case 0x57:
                case 0x5a:
                case 0x5b:
                case 0x60:
                case 0x61:
                    return true;
                    break;
                default:
                    return false;
                    break;
            }
            break;
        case 0x01:                      //AMD
            switch(pof->m_bDeviceId){
                case 0x0c:
                case 0x0f:
                case 0x23:
                case 0x34:
                case 0x4a:
                case 0xad:              //Xecuter 3
                    return true;
                    break;
                default:
                    return false;
                    break;
            }
            break;
        default:
            return false;
            break;
    }
    return false;
}

int fetchBootScriptFromFlash(u8 ** buffer){
    int bufferSize = 0;
    OBJECT_FLASH of;
    u8 * lastBlock;

    // A bit hacky, but easier to maintain.
    const KNOWN_FLASH_TYPE aknownflashtypesDefault[] = {
        #include "flashtypes.h"
    };
    if(LPCmodSettings.firstScript.ScripMagicNumber == 0xFAF1){  //No need to dig in further if no script was saved there.
        if(fHasHardware == SYSCON_ID_V1 || fHasHardware == SYSCON_ID_XT || cromwell_config==CROMWELL){

            memset(&of,0xFF,sizeof(of));
            of.m_pbMemoryMappedStartAddress=(u8 *)LPCFlashadress;

            if(BootFlashGetDescriptor(&of, (KNOWN_FLASH_TYPE *)&aknownflashtypesDefault[0])) {        //Still got flash to interface?
                bufferSize = LPCmodSettings.firstScript.nextEntryPosition - sizeof(_LPCmodSettings) - 1;
                *buffer = (u8 *)malloc(bufferSize);
                memcpy(*buffer,(const u8*)&(of.m_pbMemoryMappedStartAddress[0x3f000 + sizeof(_LPCmodSettings)]), bufferSize);
            }
        }
    }
    return bufferSize;
}
