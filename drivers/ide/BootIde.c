/*
 *  This code was originally from:
 *   UBL, The Universal Talkware Boot Loader
 *    Copyright (C) 2000 Universal Talkware Inc.
 */

int sprintf(char * buf, const char *fmt, ...);
//char * strncpy(char * dest,const char *src,int count);

#include "boot.h"
#include "shared.h"
#include "BootEEPROM.h"
#include "BootIde.h"

#undef sprintf

////////////////////////////////////
// IDE types and constants
#define IDE_SECTOR_SIZE         0x200
#define IDE_BASE1                     (0x1F0u) /* primary controller */

#define IDE_REG_EXTENDED_OFFSET       (0x200u)

#define IDE_REG_DATA(base)              ((base) + 0u) /* word register */
#define IDE_REG_ERROR(base)             ((base) + 1u)                          //Error register (read-only)
#define IDE_REG_FEATURE(base)           ((base) + 1u)                          //Features register (write-only)
#define IDE_REG_SECTOR_COUNT(base)      ((base) + 2u)                          //Sector Count register (read-write)
#define IDE_REG_SECTOR_NUMBER(base)     ((base) + 3u)                          //LBA Low register (read-write)
#define IDE_REG_LBA_LOW(base)           ((base) + 3u)                          //Same as above but easier to remember
#define IDE_REG_CYLINDER_LSB(base)      ((base) + 4u)                          //LBA Mid register (read-write)
#define IDE_REG_LBA_MID(base)           ((base) + 4u)                          //Same as above but easier to remember
#define IDE_REG_CYLINDER_MSB(base)      ((base) + 5u)                          //LBA High register (read-write)
#define IDE_REG_LBA_HIGH(base)          ((base) + 5u)                          //Same as above but easier to remember
#define IDE_REG_DRIVEHEAD(base)         ((base) + 6u)                          //Device control register (read-write)
#define IDE_REG_DEVICE(base)            ((base) + 6u)                          //Same as above but easier to remember
#define IDE_REG_STATUS(base)            ((base) + 7u)                          //Status register (read-only)
#define IDE_REG_COMMAND(base)           ((base) + 7u)                          //Command Register(write-only)
#define IDE_REG_ALTSTATUS(base)         ((base) + IDE_REG_EXTENDED_OFFSET + 6u)
#define IDE_REG_CONTROL(base)           ((base) + IDE_REG_EXTENDED_OFFSET + 6u)
#define IDE_REG_ADDRESS(base)           ((base) + IDE_REG_EXTENDED_OFFSET + 7u)

/*
Normal command input(sending to ATA device) must send the following registers:
      -Features
      -Sector Count
      -LBA Low
      -LBA Mid
      -LBA High
      -Device
      -Command

Command register must be the last one to be sent for the command to execute. It's then a good practice
to send all the necessary registers in the same order as presented above.


Normal command output(receiving from ATA device) will require a read on the following registers
    -Error
    -Sector Count
    -LBA Low
    -LBA Mid
    -LBA High
    -Device
    -Status
*/

typedef struct {
    unsigned char m_bPrecomp;
    unsigned char m_bCountSector;
    unsigned char m_bSector;
    unsigned short m_wCylinder;
    unsigned char m_bDrivehead;
    
       /* 48-bit LBA */   
    unsigned char m_bCountSectorExt;   
    unsigned char m_bSectorExt;   
    unsigned short m_wCylinderExt; 
    
#       define IDE_DH_DEFAULT (0xA0)
#       define IDE_DH_HEAD(x) ((x) & 0x0F)
#       define IDE_DH_MASTER  (0x00)
#       define IDE_DH_SLAVE   (0x10)
#       define IDE_DH_DRIVE(x) ((((x) & 1) != 0)?IDE_DH_SLAVE:IDE_DH_MASTER)
#       define IDE_DH_LBA     (0x40)
#       define IDE_DH_CHS     (0x00)

} tsIdeCommandParams;

#define IDE_DEFAULT_COMMAND { 0xFFu, 0x01, 0x00, 0x0000, IDE_DH_DEFAULT | IDE_DH_SLAVE, 0x00, 0x00, 0x0000 }
#define printk_debug bprintf


const char * const szaSenseKeys[] = {
    "No Sense", "Recovered Error", "Not Ready", "Medium Error",
    "Hardware Error", "Illegal request", "Unit Attention", "Data Protect",
    "Reserved 8", "Reserved 9", "Reserved 0xa", "Aborted Command",
    "Miscompare", "Reserved 0xf"
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//  Helper routines
//
/* -------------------------------------------------------------------------------- */

int BootIdeWaitNotBusy(unsigned uIoBase)
{
    u8 b = 0x80;                                //Start being busy
    while ((b & 0x80) && !(b & 0x08)) {         //Device is not ready until bit7(BSY) is cleared and bit3(DRQ) is set.
        b=IoInputByte(IDE_REG_ALTSTATUS(uIoBase));
    }
    return b&0x01;         //bit0 == ERR
}

/* -------------------------------------------------------------------------------- */

int BootIdeWaitDataReady(unsigned uIoBase)
{
    int i = 0x800000;
//    wait_smalldelay();

    //Assuming that the while assertion, i decrementing and if condition assertion all
    //take only a single cycle per operation(very unlikely), it would take around 34ms
    //for i to reach 0 with a CPU running at 733MHz. So no need for extra smalldelay here.
    //Since our program is single-threaded anyway, there's not much harm in polling the
    //HDD's STATUS register until the necessary state is reached.
    do {
        if (((IoInputByte(IDE_REG_ALTSTATUS(uIoBase)) & 0x88) == 0x08)){       //DRQ bit raised and BSY bit cleared.
            if(IoInputByte(IDE_REG_ALTSTATUS(uIoBase)) & 0x01) return 2;        //ERR bit raised, return 2.
            return 0;                                                           //Everything good, move on.
        }
        i--;
    } while (i != 0);

    if(IoInputByte(IDE_REG_ALTSTATUS(uIoBase)) & 0x01) return 2;                //ERR bit raised.
    return 1;                                                                   //Timeout error.
}

/* -------------------------------------------------------------------------------- */

int BootIdeIssueAtaCommand(
    unsigned uIoBase,
    ide_command_t command,
    tsIdeCommandParams * params)
{
    int n;

    //IoInputByte(IDE_REG_STATUS(uIoBase));     //No need since we'll be checking ALT_STATUS register in BootIdeWaitNotBusy function.

    n=BootIdeWaitNotBusy(uIoBase);
//    if(n)    {// as our command may be being used to clear the error, not a good policy to check too closely!
//        printk("error on BootIdeIssueAtaCommand wait 1: ret=%d, error %02X\n", n, IoInputByte(IDE_REG_ERROR(uIoBase)));
//        return 1;
//    }
     
     /* 48-bit LBA */   
        /* this won't hurt for non 48-bit LBA commands since we re-write these registers below */   
    
    IoOutputByte(IDE_REG_SECTOR_COUNT(uIoBase), params->m_bCountSectorExt);   
    IoOutputByte(IDE_REG_SECTOR_NUMBER(uIoBase), params->m_bSectorExt);   
    IoOutputByte(IDE_REG_CYLINDER_LSB(uIoBase), params->m_wCylinderExt & 0xFF);   
    IoOutputByte(IDE_REG_CYLINDER_MSB(uIoBase), (params->m_wCylinderExt >> 8) ); 
    /* End 48-bit LBA */   

    IoOutputByte(IDE_REG_SECTOR_COUNT(uIoBase), params->m_bCountSector);	//Sector count reg
    IoOutputByte(IDE_REG_SECTOR_NUMBER(uIoBase), params->m_bSector);		//LBA LOW
    IoOutputByte(IDE_REG_CYLINDER_LSB(uIoBase), params->m_wCylinder & 0xFF);	//LBA MID
    IoOutputByte(IDE_REG_CYLINDER_MSB(uIoBase), (params->m_wCylinder >> 8));	//LBA HIGH
    IoOutputByte(IDE_REG_DRIVEHEAD(uIoBase), params->m_bDrivehead);		//DEVICE REG

    IoOutputByte(IDE_REG_COMMAND(uIoBase), command);				//COMMAND REG
//    wait_smalldelay();

    n=BootIdeWaitNotBusy(uIoBase);
    if(n)    {
//        printk("\n      error on BootIdeIssueAtaCommand wait 3: ret=%d, error %02X\n", n, IoInputByte(IDE_REG_ERROR(uIoBase)));
        return 1;
    }

    return 0;
}

/* -------------------------------------------------------------------------------- */

int BootIdeReadData(unsigned uIoBase, void * buf, size_t size)
{
    u16 * ptr = (u16 *) buf;

    if (BootIdeWaitDataReady(uIoBase)) {
        //printk("BootIdeReadData data not ready...\n");
        return 1;
    }

    while (size > 1) {
        *ptr++ = IoInputWord(IDE_REG_DATA(uIoBase));
        size -= 2;
    }

    IoInputByte(IDE_REG_STATUS(uIoBase));
    if(IoInputByte(IDE_REG_ALTSTATUS(uIoBase)) & 0x01) {
        //printk("BootIdeReadData ended with an error\n");
        return 2;
    }
    return 0;
}

/* -------------------------------------------------------------------------------- */

// issues a block of data ATA-style

int BootIdeWriteData(unsigned uIoBase, void * buf, u32 size)
{
    register unsigned short * ptr = (unsigned short *) buf;
    int n;

    n=BootIdeWaitDataReady(uIoBase);
//    if(n) {
//        printk("BootIdeWriteData timeout or error from BootIdeWaitDataReady ret %d\n", n);
//        return 1;
//    }
    //wait_smalldelay();

    while (size > 1) {

        IoOutputWord(IDE_REG_DATA(uIoBase), *ptr);
        size -= 2;
        ptr++;
    }
//    wait_smalldelay();
    
    n=BootIdeWaitNotBusy(uIoBase);
    //wait_smalldelay();
    if(n) {
        //printk("BootIdeWriteData timeout or error from BootIdeWaitNotBusy ret %d\n", n);
        return 1;
    }

    if(IoInputByte(IDE_REG_ALTSTATUS(uIoBase)) & 0x01) return 2;     //ERR flag raised.
    
    return 0;
}





/* -------------------------------------------------------------------------------- */

int BootIdeWriteAtapiData(unsigned uIoBase, void * buf, size_t size)
{
    u16 * ptr = (unsigned short *) buf;
    u16 w;
    int n;

    n=BootIdeWaitDataReady(uIoBase);
//    if(n) {
//        printk("BootIdeWriteAtapiData error before data ready ret %d\n", n);
//        return 1;
//    }
    wait_smalldelay();

    w=IoInputByte(IDE_REG_CYLINDER_LSB(uIoBase));
    w|=(IoInputByte(IDE_REG_CYLINDER_MSB(uIoBase)))<<8;

//    bprintf("(bytes count =%04X)\n", w);
    n=IoInputByte(IDE_REG_STATUS(uIoBase));
    if(n&1) { // error
        //printk("BootIdeWriteAtapiData Error BEFORE writing data (0x%x bytes) err=0x%X\n", n, w);
        return 1;
    }

    while (size > 1) {

        IoOutputWord(IDE_REG_DATA(uIoBase), *ptr);
        size -= 2;
        ptr++;
    }
    n=IoInputByte(IDE_REG_STATUS(uIoBase));
    if(n&1) { // error
//        printk("BootIdeWriteAtapiData Error after writing data err=0x%X\n", n);
        return 1;
    }
    wait_smalldelay();
    n=BootIdeWaitNotBusy(uIoBase);
    if(n) {
//        printk("BootIdeWriteAtapiData timeout or error before not busy ret %d\n", n);
        return 1;
    }
    wait_smalldelay();

       if(IoInputByte(IDE_REG_ALTSTATUS(uIoBase)) & 0x01) return 2;
    
    return 0;
}


/* -------------------------------------------------------------------------------- */

int BootIdeIssueAtapiPacketCommandAndPacket(int nDriveIndex, u8 *pAtapiCommandPacket12Bytes)
{
    tsIdeCommandParams tsicp = IDE_DEFAULT_COMMAND;
    unsigned     uIoBase = tsaHarddiskInfo[nDriveIndex].m_fwPortBase;

    tsicp.m_bDrivehead = IDE_DH_DEFAULT | IDE_DH_HEAD(0) | IDE_DH_CHS | IDE_DH_DRIVE(nDriveIndex);
    IoOutputByte(IDE_REG_DRIVEHEAD(uIoBase), tsicp.m_bDrivehead);

    tsicp.m_wCylinder=2048;
    BootIdeWaitNotBusy(uIoBase);
    if(BootIdeIssueAtaCommand(uIoBase, IDE_CMD_ATAPI_PACKET, &tsicp)) 
    {
//            printk("  Drive %d: BootIdeIssueAtapiPacketCommandAndPacket 1 FAILED, error=%02X\n", nDriveIndex, IoInputByte(IDE_REG_ERROR(uIoBase)));
            return 1;
    }
    
    if(BootIdeWaitNotBusy(uIoBase)) 
    {
        //printk("  Drive %d: BootIdeIssueAtapiPacketCommandAndPacket 2 FAILED, error=%02X\n", nDriveIndex, IoInputByte(IDE_REG_ERROR(uIoBase)));
        return 1;
    }


//    printk("  Drive %d:   status=0x%02X, error=0x%02X\n",
//    nDriveIndex, IoInputByte(IDE_REG_ALTSTATUS(uIoBase)), IoInputByte(IDE_REG_ERROR(uIoBase)));

    if(BootIdeWriteAtapiData(uIoBase, pAtapiCommandPacket12Bytes, 12)) 
    {
//        printk("  Drive %d:BootIdeIssueAtapiPacketCommandAndPacket 3 FAILED, error=0x%02X\n", nDriveIndex, IoInputByte(IDE_REG_ERROR(uIoBase)));
//        BootIdeAtapiPrintkFriendlyError(nDriveIndex);
        return 1;
    }

    if(pAtapiCommandPacket12Bytes[0]!=0x1e) 
    {
        if(BootIdeWaitDataReady(uIoBase)) 
        {
            //printk("  Drive %d:  BootIdeIssueAtapiPacketCommandAndPacket Atapi Wait for data ready FAILED, status=0x%02X, error=0x%02X\n",
            //    nDriveIndex, IoInputByte(IDE_REG_ALTSTATUS(uIoBase)), IoInputByte(IDE_REG_ERROR(uIoBase)));
            return 1;
        }
    }
    return 0;
}


/* -------------------------------------------------------------------------------- */


/////////////////////////////////////////////////
//  BootIdeDriveInit
//
//  Called by BootIdeInit for each drive
//  detects and inits each drive, and the structs containing info about them

int BootIdeDriveInit(unsigned uIoBase, int nIndexDrive)
{
    tsIdeCommandParams tsicp = IDE_DEFAULT_COMMAND;
    unsigned short* drive_info;
    u8 baBuffer[512];
    u8 n = 0;
     
    tsaHarddiskInfo[nIndexDrive].m_fwPortBase = uIoBase;
    tsaHarddiskInfo[nIndexDrive].m_wCountHeads = 0u;
    tsaHarddiskInfo[nIndexDrive].m_wCountCylinders = 0u;
    tsaHarddiskInfo[nIndexDrive].m_wCountSectorsPerTrack = 0u;
    tsaHarddiskInfo[nIndexDrive].m_dwCountSectorsTotal = 1ul;
    tsaHarddiskInfo[nIndexDrive].m_bLbaMode = IDE_DH_CHS;
    tsaHarddiskInfo[nIndexDrive].m_fDriveExists = 0;
    tsaHarddiskInfo[nIndexDrive].m_enumDriveType=EDT_UNKNOWN;
    tsaHarddiskInfo[nIndexDrive].m_fAtapi=false;
    tsaHarddiskInfo[nIndexDrive].m_wAtaRevisionSupported=0;
    tsaHarddiskInfo[nIndexDrive].m_fHasMbr=0;
    tsaHarddiskInfo[nIndexDrive].m_securitySettings = 0;
    tsaHarddiskInfo[nIndexDrive].m_bIORDY = 0;

//Why disable DMA?

    tsicp.m_bDrivehead = IDE_DH_DEFAULT | IDE_DH_HEAD(0) | IDE_DH_CHS | IDE_DH_DRIVE(nIndexDrive);
    IoOutputByte(IDE_REG_DRIVEHEAD(uIoBase), tsicp.m_bDrivehead);

    IoOutputByte(IDE_REG_CONTROL(uIoBase), 0x0a); // kill interrupt,
//    IoOutputByte(IDE_REG_FEATURE(uIoBase), 0x00); // kill DMA


    if(BootIdeWaitNotBusy(uIoBase)) 
    
    {
            //printk_debug("  Drive %d: Not Ready\n", nIndexDrive);
            return 1;
    }


    
    if(BootIdeIssueAtaCommand(uIoBase, IDE_CMD_IDENTIFY, &tsicp)) {
        BootIdeIssueAtaCommand(uIoBase, ATAPI_SOFT_RESET, &tsicp);
        if (BootIdeIssueAtaCommand(uIoBase,IDE_CMD_PACKET_IDENTIFY,&tsicp)) {
            //printk(" Drive %d: Not detected\n");
            return 1;
        }
        tsaHarddiskInfo[nIndexDrive].m_fAtapi=true;
    } 
    else tsaHarddiskInfo[nIndexDrive].m_fAtapi=false;
        
    BootIdeWaitDataReady(uIoBase);
    
    if(BootIdeReadData(uIoBase, baBuffer, IDE_SECTOR_SIZE)) 
    {
        //printk("  %d: Drive not detected\n", nIndexDrive);
        return 1;
    }  
    

    drive_info = (unsigned short*)baBuffer;
    tsaHarddiskInfo[nIndexDrive].m_wCountHeads = drive_info[3];
    tsaHarddiskInfo[nIndexDrive].m_wCountCylinders = drive_info[1];
    tsaHarddiskInfo[nIndexDrive].m_wCountSectorsPerTrack = drive_info[6];
    tsaHarddiskInfo[nIndexDrive].m_dwCountSectorsTotal = *((unsigned int*)&(drive_info[60]));
    tsaHarddiskInfo[nIndexDrive].m_wAtaRevisionSupported = drive_info[88];


    VIDEO_ATTR=0xffc8c8c8;
        /* 48-bit LBA - we should check bits 83 AND 86 to check both
     * supported AND enabled  - however, some drives do not set
     * Bit 83. Bit 86 seems to be the accepted way to detect whether
     * 48-bit LBA is available. */
        if( drive_info[86] & 1ul<<10 )  {
                //if (!(drive_info[83] & 1ul<<10))
            //printk("Warning - ATA Bit 83 is not set - attempting LBA48 anyway\n");
    
        tsaHarddiskInfo[nIndexDrive].m_dwCountSectorsTotal = 
            *((unsigned int*)&(drive_info[100]));
    }
    /* End 48-bit LBA */   
    
    { 
        u16 * pw=(u16 *)&(drive_info[10]);
        tsaHarddiskInfo[nIndexDrive].s_length =
            copy_swap_trim(tsaHarddiskInfo[nIndexDrive].m_szSerial,(u8*)pw,20);
        pw=(u16 *)&(drive_info[27]);
        tsaHarddiskInfo[nIndexDrive].m_length =
            copy_swap_trim(tsaHarddiskInfo[nIndexDrive].m_szIdentityModelNumber,(u8 *)pw,40);
        copy_swap_trim(tsaHarddiskInfo[nIndexDrive].m_szFirmware,(u8 *)&(drive_info[23]),8);

    }

    tsaHarddiskInfo[nIndexDrive].m_fDriveExists = 1;

    if (tsaHarddiskInfo[nIndexDrive].m_fAtapi) {
     // CDROM/DVD
                // We Detected a CD-DVD or so, as there are no Heads ...
        tsaHarddiskInfo[nIndexDrive].m_fAtapi=true;
#ifndef SILENT_MODE
        printk("hd%c: ", nIndexDrive+'a');
        VIDEO_ATTR=0xffc8c800;

        printk("%s %s %s - ATAPI\n",tsaHarddiskInfo[nIndexDrive].m_szIdentityModelNumber,
            tsaHarddiskInfo[nIndexDrive].m_szIdentityModelNumber,
            tsaHarddiskInfo[nIndexDrive].m_szSerial,
            tsaHarddiskInfo[nIndexDrive].m_szFirmware);
#endif

        if (cromwell_config==CROMWELL) 
        {
              // this is the only way to clear the ATAPI ''I have been reset'' error indication
            u8 ba[128];
            ba[2]=0x06;
            while (ba[2]==0x06) 
            {  
                // while bitching that it 'needs attention', give it REQUEST SENSE
                int nPacketLength=BootIdeAtapiAdditionalSenseCode(nIndexDrive, ba, sizeof(ba));
                if(nPacketLength<12) 
                {
                    //printk("Unable to get ASC from drive when clearing sticky DVD error, retcode=0x%x\n", nPacketLength);
                    ba[2]=0;
                } else {
                    //printk("ATAPI Drive reports ASC 0x%02X\n", ba[12]);  // normally 0x29 'reset' but clears the condition by reading
                }
                  }
        }
    } 
        
        
    if (!tsaHarddiskInfo[nIndexDrive].m_fAtapi) {       //Drive is HDD (not CD/DVD).
        unsigned long ulDriveCapacity1024=((tsaHarddiskInfo[nIndexDrive].m_dwCountSectorsTotal /1000)*512)/1000;
        
#ifndef SILENT_MODE
        printk("hd%c: ", nIndexDrive+'a');
        VIDEO_ATTR=0xffc8c800;

        printk("%s %s %u.%uGB - HDD\n",
            tsaHarddiskInfo[nIndexDrive].m_szIdentityModelNumber,
            tsaHarddiskInfo[nIndexDrive].m_szFirmware,
            ulDriveCapacity1024/1000, ulDriveCapacity1024%1000 
        );
#endif
        tsaHarddiskInfo[nIndexDrive].m_securitySettings = drive_info[128];
        
        if (cromwell_config==CROMWELL) {
            if((drive_info[128]&0x0004)==0x0004) 
            { 
                unsigned char password[20];
                if (CalculateDrivePassword(nIndexDrive,password)) {
                    printk("Unable to calculate drive password - eeprom corrupt?");
                    return 1;
                }
                
                if (DriveSecurityChange(uIoBase, nIndexDrive, IDE_CMD_SECURITY_UNLOCK, password)) {
                    printk("Unlock failed!");
                }
                else {
#ifndef SILENT_MODE
                    printk("Unlock OK");    
#endif
                }
            }
        }  

        //Useful for READ/WRITE_MLTIPLE commands
        tsaHarddiskInfo[nIndexDrive].m_maxBlockTransfer = *((unsigned char*)&(drive_info[47]));

        //ATA specs require that we send SET MULTIPLE MOD command at least once to enable MULTIPLE READ/WRITE
        //commands. We send the SET MULTIPLE MODE command with the default number of sector per block.
        //But first let's check if we need to. 
        //If bit8 of word 59 is set along with a valid sector/block value, we don't need to send command.
        if((tsaHarddiskInfo[nIndexDrive].m_maxBlockTransfer + 0x0100) != (drive_info[59]&0x01FF)){
            if(BootIdeSetMultimodeSectors(nIndexDrive, tsaHarddiskInfo[nIndexDrive].m_maxBlockTransfer)){
                printk("\n\n\n\n\n\n\n\n              Unable to change Multimode's sectors per block.");
                wait_ms(3000);
                return 1;
            }
        }
        if (drive_info[49] & 0x400) 
        { 
            /* bit 10 of capability word is IORDY supported bit */
            tsaHarddiskInfo[nIndexDrive].m_bIORDY = 1;
        }

        tsaHarddiskInfo[nIndexDrive].m_minPIOcycle = *((unsigned int*)&(drive_info[67]));       //Value in ns
        //Set fastest PIO mode depending on cycle time supplied by HDD.

        //Depending on PIO cycle time value returned by IDENTIFY command, we select a PIO mode.
        //Can be from 0 to 4. One thing important is that bit3 must be set to 1(0x08).
        //Bits 2 to 0 select the PIO mode.
        if(tsaHarddiskInfo[nIndexDrive].m_minPIOcycle <= 120
              && tsaHarddiskInfo[nIndexDrive].m_bIORDY)        //Mode4
            n = BootIdeSetTransferMode(nIndexDrive, 0x0C);                                                                                                                   
        else if(tsaHarddiskInfo[nIndexDrive].m_minPIOcycle <= 180
                   && tsaHarddiskInfo[nIndexDrive].m_bIORDY)   //Mode3
            n = BootIdeSetTransferMode(nIndexDrive, 0x0B);
        if(tsaHarddiskInfo[nIndexDrive].m_minPIOcycle <= 240)   //Mode2
            n = BootIdeSetTransferMode(nIndexDrive, 0x0A);
        else if(tsaHarddiskInfo[nIndexDrive].m_minPIOcycle <= 383)   //Mode1
            n = BootIdeSetTransferMode(nIndexDrive, 0x09);
        else                                                         //Mode0
            n = BootIdeSetTransferMode(nIndexDrive, 0x08);
        if(n){
            printk("\n       BootIdeSetPIOMode:Drive %d: Cannot set PIO mode.", nIndexDrive);
        }

//We'll give PIO Mode4 a shot first.
/*
        if(BootIdeSetTransferMode(nIndexDrive, 0x40 | 2)){  //Enable UDMA2 for drive. Best mode for 40 conductors cable.
                                                            //I don't care if you have a 80 conductors for now.
            printk("\n       Unable to init DMA for drive %u", nIndexDrive);
        }
        else
            tsaHarddiskInfo[nIndexDrive].m_fDMAInit=1;      //1 = Hardware init done.

        //Next steps in DMA would be to send a IDE_CMD_SET_FEATURES command selecting
        //a DMA transfer mode.
        //After, we need to set up a PRD table and send it to the hdd's controller.
        //Then we would be ready to put some data in memory and send DMA R/W commands to HDD.
*/
    }
    if (drive_info[49] & 0x200) 
    { 
        /* bit 9 of capability word is lba supported bit */
        tsaHarddiskInfo[nIndexDrive].m_bLbaMode = IDE_DH_LBA;
    } else {
        tsaHarddiskInfo[nIndexDrive].m_bLbaMode = IDE_DH_CHS;
    }

    //If drive is a hard disk, see what type of partitioning it has.
    if (!tsaHarddiskInfo[nIndexDrive].m_fAtapi) {

        unsigned char ba[512];
        int nError;

        if((nError=BootIdeReadSector(nIndexDrive, ba, 3, 0, 512))) 
        {
            //printk("  -  Unable to read FATX sector");
        } else {
            if( (ba[0]=='B') && (ba[1]=='R') && (ba[2]=='F') && (ba[3]=='R') ) 
            {
                tsaHarddiskInfo[nIndexDrive].m_enumDriveType=EDT_XBOXFS;
#ifndef SILENT_MODE
                printk(" - FATX", nIndexDrive);
            } else {
                printk(" - No FATX", nIndexDrive);
#endif
            }
        }

            // report on the MBR-ness of the drive contents

        if(FATXCheckMBR(nIndexDrive)) {
#ifndef SILENT_MODE
            printk(" - MBR", nIndexDrive);
#endif
            tsaHarddiskInfo[nIndexDrive].m_fHasMbr=1;
        }
        else {
            tsaHarddiskInfo[nIndexDrive].m_fHasMbr=0;
#ifndef SILENT_MODE
            printk(" - No MBR", nIndexDrive);
#endif
        }
        printk("\n");
    } 

    return 0;
}




/* -------------------------------------------------------------------------------- */
int DriveSecurityChange(unsigned uIoBase, int driveId, ide_command_t ide_cmd, unsigned char *password) {
    //Todo: Check drive is in correct state for command desired.
    char ide_cmd_data[2+512];    
    char baBuffer[512];
    unsigned short*    drive_info = (unsigned short*)baBuffer;
    tsIdeCommandParams tsicp = IDE_DEFAULT_COMMAND;
    tsIdeCommandParams tsicp1 = IDE_DEFAULT_COMMAND;

    
    if(BootIdeWaitNotBusy(uIoBase)) 
    {
        //printk("  %d:  Not Ready\n", driveId);
        return 1;
    }
    tsicp1.m_bDrivehead = IDE_DH_DEFAULT | IDE_DH_HEAD(0) | IDE_DH_CHS | IDE_DH_DRIVE(driveId);

    if (ide_cmd == IDE_CMD_SECURITY_SET_PASSWORD) {
        //Cromwell locks drives in high security mode (NOT maximum security mode).
        //This means that even if you lose the normal (user) password, you can
        //unlock them again using the master password set below.
        //Master password is TEAMASSEMBLY (in ascii, NULL padded)
        char *master_password="TEAMASSEMBLY";
        
        //We first lock the drive with the master password
        //Just in case we ever need to unlock it in an emergency
        memset(ide_cmd_data,0x00,512);
        //Set master password flag
        ide_cmd_data[0]|=0x01;
    
        memcpy(&ide_cmd_data[2],master_password,12);
        
        if(BootIdeIssueAtaCommand(uIoBase, ide_cmd, &tsicp1)) return 1;
        BootIdeWaitDataReady(uIoBase);
        BootIdeWriteData(uIoBase, ide_cmd_data, IDE_SECTOR_SIZE);

        BootIdeWaitNotBusy(uIoBase);
    }

    memset(ide_cmd_data,0x00,512);
    
    //Password is only 20 bytes long - the rest is 0-padded.
    memcpy(&ide_cmd_data[2],password,20);

    if(BootIdeIssueAtaCommand(uIoBase, ide_cmd, &tsicp1)) return 1;
        
    BootIdeWaitDataReady(uIoBase);
    BootIdeWriteData(uIoBase, ide_cmd_data, IDE_SECTOR_SIZE);
           
    if (BootIdeWaitNotBusy(uIoBase))    
    {
        return 1;
    }
    // check that we are unlocked
    tsicp.m_bDrivehead = IDE_DH_DEFAULT | IDE_DH_HEAD(0) | IDE_DH_CHS | IDE_DH_DRIVE(driveId);
    if(BootIdeIssueAtaCommand(uIoBase, IDE_CMD_IDENTIFY, &tsicp)) 
    {
        return 1;
    }
    BootIdeWaitDataReady(uIoBase);
    if(BootIdeReadData(uIoBase, baBuffer, IDE_SECTOR_SIZE)) 
    {
        return 1;
    }

    tsaHarddiskInfo[driveId].m_securitySettings = drive_info[128];
    //Success, hopefully.
    return 0;
}

int CalculateDrivePassword(int driveId, unsigned char *key) {

    u8 baMagic[0x200], baKeyFromEEPROM[0x10], baEeprom[0x30];
    int nVersionHashing=0;
    //Ick - forward decl. Should remove this. 
    u32 BootHddKeyGenerateEepromKeyData(u8 *eeprom_data,u8 *HDKey);
    
    memcpy(baEeprom, &eeprom, 0x30); // first 0x30 bytes from EEPROM image we picked up earlier

    memset(&baKeyFromEEPROM,0x00,0x10);
    nVersionHashing = BootHddKeyGenerateEepromKeyData( baEeprom, baKeyFromEEPROM);
    memset(&baMagic,0x00,0x200);
    // Calculate the hdd pw from EEprom and Serial / Model Number
    HMAC_SHA1 (&baMagic[2], baKeyFromEEPROM, 0x10,
         tsaHarddiskInfo[driveId].m_szIdentityModelNumber,
         tsaHarddiskInfo[driveId].m_length,
         tsaHarddiskInfo[driveId].m_szSerial,
         tsaHarddiskInfo[driveId].s_length);

    //Failed to generate a key
    if (nVersionHashing==0) return 1;

    memcpy(key,&baMagic[2],20);
    return 0;
}


/////////////////////////////////////////////////
//  BootIdeInit
//
//  Called at boot-time to init and detect connected devices

int BootIdeInit(void)
{
    memset(&tsaHarddiskInfo[0],0x00,sizeof(struct tsHarddiskInfo));
    memset(&tsaHarddiskInfo[1],0x00,sizeof(struct tsHarddiskInfo));
    
    tsaHarddiskInfo[0].m_bCableConductors=40;
    tsaHarddiskInfo[1].m_bCableConductors=40;
    tsaHarddiskInfo[0].m_fDMAInit=0;            //DMA not initialized yet.
    tsaHarddiskInfo[1].m_fDMAInit=0;
//    IoOutputByte(0xff60+0, 0x00); // stop bus mastering
    IoOutputByte(0xff60+2, 0x62); // DMA possible for both drives

    //Init both master and slave
    BootIdeDriveInit(IDE_BASE1, 0);
    BootIdeDriveInit(IDE_BASE1, 1);
       
        
    if(tsaHarddiskInfo[0].m_fDriveExists) 
    {
        unsigned int uIoBase = tsaHarddiskInfo[0].m_fwPortBase;
        tsIdeCommandParams tsicp = IDE_DEFAULT_COMMAND;

        tsicp.m_bDrivehead = IDE_DH_DEFAULT | IDE_DH_HEAD(0) | IDE_DH_CHS | IDE_DH_DRIVE(0);
        IoOutputByte(IDE_REG_DRIVEHEAD(uIoBase), tsicp.m_bDrivehead);

        if(!BootIdeIssueAtaCommand(uIoBase, IDE_CMD_IDENTIFY, &tsicp)) 
        {
            u16 waBuffer[256];
            BootIdeWaitDataReady(uIoBase);
            if(!BootIdeReadData(uIoBase, (u8 *)&waBuffer[0], IDE_SECTOR_SIZE)) 
            {
//                printk("%04X ", waBuffer[80]);
                if( ((waBuffer[93]&0xc000)!=0) && ((waBuffer[93]&0x8000)==0) && ((waBuffer[93]&0xe000)!=0x6000))     
                {
                    tsaHarddiskInfo[0].m_bCableConductors=80;
                }

            } else {
                //printk("Error getting final GET_INFO\n");
            }
        }
    }

    if(tsaHarddiskInfo[0].m_bCableConductors==40) 
    {
#ifndef SILENT_MODE
        printk("UDMA2\n");
#endif
    } else 
    {
        int nAta=0;
        if(tsaHarddiskInfo[0].m_wAtaRevisionSupported&2) nAta=1;
        if(tsaHarddiskInfo[0].m_wAtaRevisionSupported&4) nAta=2;
        if(tsaHarddiskInfo[0].m_wAtaRevisionSupported&8) nAta=3;
        if(tsaHarddiskInfo[0].m_wAtaRevisionSupported&16) nAta=4;
        if(tsaHarddiskInfo[0].m_wAtaRevisionSupported&32) nAta=5;
#ifndef SILENT_MODE
        printk("UDMA%d\n", nAta);
#endif
    }

    return 0;
}

/* -------------------------------------------------------------------------------- */

/////////////////////////////////////////////////
//  BootIdeAtapiModeSense
//
//  returns the ATAPI extra error info block
/*
int BootIdeAtapiModeSense(int nDriveIndex, u8 bCodePage, u8 * pba, int nLengthMaxReturn) 
{
    unsigned uIoBase = tsaHarddiskInfo[nDriveIndex].m_fwPortBase;

    u8 ba[2048];
    int nReturn;

    if(!tsaHarddiskInfo[nDriveIndex].m_fDriveExists) return 4;

    memset(ba, 0, sizeof(ba));
    //memset(&ba[0], 0, 12);
    ba[0]=0x5a;
    ba[2]=bCodePage;
    ba[7]=(u8)(sizeof(ba)>>8); 
    ba[8]=(u8)sizeof(ba);

    if(BootIdeIssueAtapiPacketCommandAndPacket(nDriveIndex, ba)) 
    {
//            u8 bStatus=IoInputByte(IDE_REG_ALTSTATUS(uIoBase)), bError=IoInputByte(IDE_REG_ERROR(uIoBase));
//            printk("  Drive %d: BootIdeAtapiAdditionalSenseCode FAILED, status=%02X, error=0x%02X, ASC unavailable\n", nDriveIndex, bStatus, bError);
            return 1;
    }

    nReturn=IoInputByte(IDE_REG_CYLINDER_LSB(uIoBase));
    nReturn |=IoInputByte(IDE_REG_CYLINDER_MSB(uIoBase))<<8;
    if(nReturn>nLengthMaxReturn) nReturn=nLengthMaxReturn;
    BootIdeReadData(uIoBase, pba, nReturn);

    return nReturn;
}
*
/* -------------------------------------------------------------------------------- */

/////////////////////////////////////////////////
//  BootIdeAtapiAdditionalSenseCode
//
//  returns the ATAPI extra error info block

int BootIdeAtapiAdditionalSenseCode(int nDriveIndex, u8 * pba, int nLengthMaxReturn) 
{
    unsigned uIoBase = tsaHarddiskInfo[nDriveIndex].m_fwPortBase;

    u8 ba[2048];
    int nReturn;

    if(!tsaHarddiskInfo[nDriveIndex].m_fDriveExists) return 4;

    //memset(&ba[0], 0, 12);
    memset(ba, 0, sizeof(ba));
    ba[0]=0x03;
    ba[4]=0xfe;

    if(BootIdeIssueAtapiPacketCommandAndPacket(nDriveIndex, ba)) 
    {
            return 1;
    }

    nReturn=IoInputByte(IDE_REG_CYLINDER_LSB(uIoBase));
    nReturn |=IoInputByte(IDE_REG_CYLINDER_MSB(uIoBase))<<8;
    if(nReturn>nLengthMaxReturn) nReturn=nLengthMaxReturn;
    BootIdeReadData(uIoBase, pba, nReturn);

    return nReturn;
}
/* -------------------------------------------------------------------------------- */

bool BootIdeAtapiReportFriendlyError(int nDriveIndex, char * szErrorReturn, int nMaxLengthError)
{
    u8 ba[2048];
    char szError[512];
    int nReturn;
    bool f=true;

    memset(ba, 0, sizeof(ba));
    nReturn=BootIdeAtapiAdditionalSenseCode(nDriveIndex, ba, sizeof(ba));
    if(nReturn<12) {
        sprintf(szError, "Unable to get Sense Code\n");
        f=false;
    } else {
        sprintf(szError, "Sense key 0x%02X (%s), ASC=0x%02X, qualifier=0x%02X\n", ba[2]&0x0f, szaSenseKeys[ba[2]&0x0f], ba[12], ba[13]);
        VideoDumpAddressAndData(0, ba, nReturn);
    }

    strncpy(szErrorReturn, szError, nMaxLengthError);
    return f;
}

/* -------------------------------------------------------------------------------- */
/*
void BootIdeAtapiPrintkFriendlyError(int nDriveIndex)
{
    char sz[512];
    memset(&sz,0x00,sizeof(sz));
    BootIdeAtapiReportFriendlyError(nDriveIndex, sz, sizeof(sz));
    printk(sz);
}
*/

/* -------------------------------------------------------------------------------- */

/////////////////////////////////////////////////
//  BootIdeReadSector
//
//  Read an absolute sector from the device
//  knows if it should use ATA or ATAPI according to HDD or DVD
//  This is the main function for reading things from a drive

int BootIdeReadSector(int nDriveIndex, void * pbBuffer, unsigned int block, int byte_offset, int n_bytes) 
{
    tsIdeCommandParams tsicp = IDE_DEFAULT_COMMAND;
    unsigned uIoBase;
    unsigned char baBufferSector[IDE_SECTOR_SIZE];
    unsigned int track;
    int status;
    unsigned char ideReadCommand = IDE_CMD_READ_MULTI_RETRY; /* 48-bit LBA */
    
    if(!tsaHarddiskInfo[nDriveIndex].m_fDriveExists) return 4;

    uIoBase = tsaHarddiskInfo[nDriveIndex].m_fwPortBase;

    tsicp.m_bDrivehead = IDE_DH_DEFAULT | IDE_DH_HEAD(0) | IDE_DH_CHS | IDE_DH_DRIVE(nDriveIndex);
    IoOutputByte(IDE_REG_DRIVEHEAD(uIoBase), tsicp.m_bDrivehead);

    if ((nDriveIndex < 0) || (nDriveIndex >= 2) ||
        (tsaHarddiskInfo[nDriveIndex].m_fDriveExists == 0))
    {
        //printk("unknown drive\n");
        return 1;
    }

    if(tsaHarddiskInfo[nDriveIndex].m_fAtapi) 
    {
                   // CD - DVD ROM
        u8 ba[12];
        int nReturn;

        IoInputByte(IDE_REG_STATUS(uIoBase));
        if(IoInputByte(IDE_REG_STATUS(uIoBase)&1)) 
        {     // sticky error
            if(IoInputByte(IDE_REG_ERROR(uIoBase)&0x20)) 
            {     // needs attention
                if(BootIdeAtapiAdditionalSenseCode(nDriveIndex, ba, 2048)<12) 
                {     // needed as it clears NEED ATTENTION
//                    printk("BootIdeReadSector sees unit needs attention but failed giving it\n");
                } else {
//                    printk("BootIdeReadSector sees unit needs attention, gave it, current Error=%02X\n", IoInputByte(IDE_REG_ERROR(uIoBase)));
                }
            }
        }

        BootIdeWaitNotBusy(uIoBase);

        if(n_bytes<2048) 
        {
            //printk("Error for drive %i: Must have 2048 byte sector for ATAPI!!!!!\n",nDriveIndex);
            return 1;
        }

        tsicp.m_wCylinder=2048;
        memset(ba, 0, sizeof(ba));
        //memset(&ba[0], 0, 12);
        ba[0]=0x28; 
        ba[2]=block>>24; 
        ba[3]=block>>16; 
        ba[4]=block>>8; 
        ba[5]=block; 
        ba[7]=0; 
        ba[8]=1;

        if(BootIdeIssueAtapiPacketCommandAndPacket(nDriveIndex, ba)) 
        {
//            printk("BootIdeReadSector Unable to issue ATAPI command\n");
            return 1;
        }

//        printk("BootIdeReadSector issued ATAPI command\n");

        nReturn=IoInputByte(IDE_REG_CYLINDER_LSB(uIoBase));
        nReturn |=IoInputByte(IDE_REG_CYLINDER_MSB(uIoBase))<<8;
//        printk("nReturn = %x\n", nReturn);

        if(nReturn>2048) nReturn=2048;
        status = BootIdeReadData(uIoBase, pbBuffer, nReturn);
        if (status != 0) 
        {
            while(1) 
            {
                wait_ms(50);
                status = BootIdeReadData(uIoBase, pbBuffer, nReturn);                
                if (status == 0) 
                {
                    break;
                }
            }
        }
        return 0;
    }

    if (tsaHarddiskInfo[nDriveIndex].m_wCountHeads > 8) 
    {
        IoOutputByte(IDE_REG_CONTROL(uIoBase), 0x0a);
    } else {
        IoOutputByte(IDE_REG_CONTROL(uIoBase), 0x02);
    }

    tsicp.m_bCountSector = 1;

    
    
    if( block >= 0x10000000 ) 
    {     
        /* 48-bit LBA access required for this block */ 
        
        tsicp.m_bCountSectorExt = 0;   
        
         /* This routine can have a max LBA of 32 bits (due to unsigned int data type used for block parameter) */   
        
        tsicp.m_wCylinderExt = 0; /* 47:32 */   
        tsicp.m_bSectorExt = (block >> 24) & 0xff; /* 31:24 */   
        tsicp.m_wCylinder = (block >> 8) & 0xffff; /* 23:8 */   
        tsicp.m_bSector = block & 0xff; /* 7:0 */   
        tsicp.m_bDrivehead = IDE_DH_DRIVE(nDriveIndex) | IDE_DH_LBA;   
        ideReadCommand = IDE_CMD_READ_EXT;   
    
    } else {
            // Looks Like we do not have LBA 48 need
            if (tsaHarddiskInfo[nDriveIndex].m_bLbaMode == IDE_DH_CHS) 
            { 

            track = block / tsaHarddiskInfo[nDriveIndex].m_wCountSectorsPerTrack;
            
            tsicp.m_bSector = 1+(block % tsaHarddiskInfo[nDriveIndex].m_wCountSectorsPerTrack);
            tsicp.m_wCylinder = track / tsaHarddiskInfo[nDriveIndex].m_wCountHeads;
            tsicp.m_bDrivehead = IDE_DH_DEFAULT |
                IDE_DH_HEAD(track % tsaHarddiskInfo[nDriveIndex].m_wCountHeads) |
                IDE_DH_DRIVE(nDriveIndex) |
                IDE_DH_CHS;
        } else {

            tsicp.m_bSector = block & 0xff; /* lower byte of block (lba) */
            tsicp.m_wCylinder = (block >> 8) & 0xffff; /* middle 2 bytes of block (lba) */
            tsicp.m_bDrivehead = IDE_DH_DEFAULT | /* set bits that must be on */
                ((block >> 24) & 0x0f) | /* lower nibble of byte 3 of block */
                IDE_DH_DRIVE(nDriveIndex) |
                IDE_DH_LBA;
        }
        }       
        
    if(BootIdeIssueAtaCommand(uIoBase, ideReadCommand, &tsicp)) 
    {
        //printk("ide error %02X...\n", IoInputByte(IDE_REG_ERROR(uIoBase)));
        return 1;
    }
    
    if (n_bytes != IDE_SECTOR_SIZE)
    {
        status = BootIdeReadData(uIoBase, baBufferSector, IDE_SECTOR_SIZE);
        if (status == 0) {
            memcpy(pbBuffer, baBufferSector+byte_offset, n_bytes);
        
        } else {
            // UPS, it failed, but we are brutal, we try again ....
            while(1) {
                wait_ms(50);
                status = BootIdeReadData(uIoBase, baBufferSector, IDE_SECTOR_SIZE);
                if (status == 0) {
                    memcpy(pbBuffer, baBufferSector+byte_offset, n_bytes);
                    break;
                }
            }
            
        }
    
    } else {
    
        status = BootIdeReadData(uIoBase, pbBuffer, IDE_SECTOR_SIZE);
        if (status!=0) {
            // UPS, it failed, but we are brutal, we try again ....
            while(1) {
                wait_ms(50);
                status = BootIdeReadData(uIoBase, pbBuffer, IDE_SECTOR_SIZE);        
                if (status == 0) {
                    break;
                }
            }
        }
    }
    return status;
}

/* -------------------------------------------------------------------------------- */



/////////////////////////////////////////////////
//  BootIdeWriteSector
//
// !!!!! EXPERIMENTAL

int BootIdeWriteSector(int nDriveIndex, void * pbBuffer, unsigned int block, u8 retry)
{
    tsIdeCommandParams tsicp = IDE_DEFAULT_COMMAND;
    unsigned uIoBase;
    unsigned int track;
    int status;
    unsigned char ideWriteCommand = IDE_CMD_WRITE_MULTI_RETRY; 
    
    if(!tsaHarddiskInfo[nDriveIndex].m_fDriveExists) return 4;

    uIoBase = tsaHarddiskInfo[nDriveIndex].m_fwPortBase;

    tsicp.m_bDrivehead = IDE_DH_DEFAULT | IDE_DH_HEAD(0) | IDE_DH_CHS | IDE_DH_DRIVE(nDriveIndex);
    IoOutputByte(IDE_REG_DRIVEHEAD(uIoBase), tsicp.m_bDrivehead);

    if ((nDriveIndex < 0) || (nDriveIndex >= 2))
    {
        //printk("unknown drive\n");
        return 1;
    }

    if (tsaHarddiskInfo[nDriveIndex].m_wCountHeads > 8) 
    {
        IoOutputByte(IDE_REG_CONTROL(uIoBase), 0x0a);
    } else {
        IoOutputByte(IDE_REG_CONTROL(uIoBase), 0x02);
    }

    tsicp.m_bCountSector = 1;
    
    if( block >= 0x10000000 ) 
    {     
        /* 48-bit LBA access required for this block */ 
        tsicp.m_bCountSectorExt = 0;   
        
         /* This routine can have a max LBA of 32 bits (due to unsigned int data type used for block parameter) */   
        tsicp.m_wCylinderExt = 0; /* 47:32 */   
        tsicp.m_bSectorExt = (block >> 24) & 0xff; /* 31:24 */   
        tsicp.m_wCylinder = (block >> 8) & 0xffff; /* 23:8 */   
        tsicp.m_bSector = block & 0xff; /* 7:0 */   
        tsicp.m_bDrivehead = IDE_DH_DRIVE(nDriveIndex) | IDE_DH_LBA;   
        ideWriteCommand = IDE_CMD_WRITE_EXT;
    } else {
            // Looks Like we do not have LBA 48 need
            if (tsaHarddiskInfo[nDriveIndex].m_bLbaMode == IDE_DH_CHS) 
            { 

            track = block / tsaHarddiskInfo[nDriveIndex].m_wCountSectorsPerTrack;
            
            tsicp.m_bSector = 1+(block % tsaHarddiskInfo[nDriveIndex].m_wCountSectorsPerTrack);
            tsicp.m_wCylinder = track / tsaHarddiskInfo[nDriveIndex].m_wCountHeads;
            tsicp.m_bDrivehead = IDE_DH_DEFAULT |
                IDE_DH_HEAD(track % tsaHarddiskInfo[nDriveIndex].m_wCountHeads) |
                IDE_DH_DRIVE(nDriveIndex) |
                IDE_DH_CHS;
        } else {

            tsicp.m_bSector = block & 0xff; /* lower byte of block (lba) */
            tsicp.m_wCylinder = (block >> 8) & 0xffff; /* middle 2 bytes of block (lba) */
            tsicp.m_bDrivehead = IDE_DH_DEFAULT | /* set bits that must be on */
                ((block >> 24) & 0x0f) | /* lower nibble of byte 3 of block */
                IDE_DH_DRIVE(nDriveIndex) |
                IDE_DH_LBA;
        }
        }       
    if(BootIdeIssueAtaCommand(uIoBase, ideWriteCommand, &tsicp)) 
    {
        printk("ide error %02X...\n", IoInputByte(IDE_REG_ERROR(uIoBase)));
        return 1;
    }
    status = BootIdeWriteData(uIoBase, pbBuffer, IDE_SECTOR_SIZE);

    if(retry > 0)
        retry -= 1;
    if(status && retry){ //Status set to 1 or 2 means error. retry count must not be 0.
        printk("\n                 BootIdeWriteSector: write sector failed. %u retry left.", retry);
        status = BootIdeWriteSector(nDriveIndex, pbBuffer, block, retry);      //Retry one more time.
    }
/*
    //Some drives requires a CACHE_FLUSH command being sent after each write command. We do it just to be sure.
    //So issue command. BSY has been cleared in BootIdeWriteData function
    tsicp = IDE_DEFAULT_COMMAND;
    //tsicp.m_bDrivehead = IDE_DH_DEFAULT | IDE_DH_HEAD(0) | IDE_DH_CHS | IDE_DH_DRIVE(nDriveIndex);
    if(tsaHarddiskInfo[nDriveIndex].m_dwCountSectorsTotal >= 0x10000000)        //LBA48 drive
        BootIdeIssueAtaCommand(uIoBase, IDE_CMD_CACHE_FLUSH_EXT, &tsicp);
    else
        BootIdeIssueAtaCommand(uIoBase, IDE_CMD_CACHE_FLUSH, &tsicp);
*/
    return status;
}

/* -------------------------------------------------------------------------------- */



/////////////////////////////////////////////////
//  BootIdeWriteMultiple
//
//  Write a block of sector in a single command instead of sector by sector.
//  This command will make the drive generate an IRQ only a the end of the block
//  Hard drives usually allow 16 sectors per blocks.
//
// !!!!! EXPERIMENTAL
// "pbBuffer" must be of size specified by "len"
// "len" is the size in sectors. For now support up to 256 sectors per issued command. 0 = 256.
// "StartLBA is the absolute LBA value of the start of the block
// "retry" is the number of time to try to write in the event the command would fail the first time.
//              Value of 3 will write once and retry 2 times if previously failed.

int BootIdeWriteMultiple(int nDriveIndex, void * pbBuffer, unsigned int startLBA, u8 len, u8 retry)
{
    tsIdeCommandParams tsicp = IDE_DEFAULT_COMMAND;
    u16 remainingLen = (len == 0)? 256 : len;   //Set remainingLen to 256 if len == 0.
    u16 nbBlocks = remainingLen / tsaHarddiskInfo[nDriveIndex].m_maxBlockTransfer;    //Calculated number of blocks to transfer.
    u8 partialBlock = len % tsaHarddiskInfo[nDriveIndex].m_maxBlockTransfer;     //Size in sector of partial block.
    unsigned uIoBase;
    unsigned int track, bufferPtr = 0;
    int status;
    unsigned char ideWriteCommand = IDE_CMD_WRITE_MULTIPLE;

    if(!tsaHarddiskInfo[nDriveIndex].m_fDriveExists) return 4;

    uIoBase = tsaHarddiskInfo[nDriveIndex].m_fwPortBase;

    tsicp.m_bDrivehead = IDE_DH_DEFAULT | IDE_DH_HEAD(0) | IDE_DH_CHS | IDE_DH_DRIVE(nDriveIndex);
    IoOutputByte(IDE_REG_DRIVEHEAD(uIoBase), tsicp.m_bDrivehead);

    if ((nDriveIndex < 0) || (nDriveIndex >= 2))
    {
        printk("\n               unknown drive\n");
        return 1;
    }

    if (tsaHarddiskInfo[nDriveIndex].m_wCountHeads > 8)
    {
        IoOutputByte(IDE_REG_CONTROL(uIoBase), 0x0a);
    } else {
        IoOutputByte(IDE_REG_CONTROL(uIoBase), 0x02);
    }

    tsicp.m_bCountSector = len; //0 = 256 sectors.

    if((startLBA + len) >= 0x10000000 )
    {
        /* 48-bit LBA access required for this block */
        tsicp.m_bCountSectorExt = 0;

         /* This routine can have a max LBA of 32 bits (due to unsigned int data type used for block parameter) */
        tsicp.m_wCylinderExt = 0; /* 47:32 */
        tsicp.m_bSectorExt = (startLBA >> 24) & 0xff; /* 31:24 */
        tsicp.m_wCylinder = (startLBA >> 8) & 0xffff; /* 23:8 */
        tsicp.m_bSector = startLBA & 0xff; /* 7:0 */
        tsicp.m_bDrivehead = IDE_DH_DRIVE(nDriveIndex) | IDE_DH_LBA;
        ideWriteCommand = IDE_CMD_WRITE_MULTIPLE_EXT;
    } else {
            // Looks Like we do not have LBA 48 need
            if (tsaHarddiskInfo[nDriveIndex].m_bLbaMode == IDE_DH_CHS)          //CHS mode, unlikely
            {

            track = startLBA / tsaHarddiskInfo[nDriveIndex].m_wCountSectorsPerTrack;

            tsicp.m_bSector = 1+(startLBA % tsaHarddiskInfo[nDriveIndex].m_wCountSectorsPerTrack);
            tsicp.m_wCylinder = track / tsaHarddiskInfo[nDriveIndex].m_wCountHeads;
            tsicp.m_bDrivehead = IDE_DH_DEFAULT |
                IDE_DH_HEAD(track % tsaHarddiskInfo[nDriveIndex].m_wCountHeads) |
                IDE_DH_DRIVE(nDriveIndex) |
                IDE_DH_CHS;
            } else {                                                            //LBA28 mode

                tsicp.m_bSector = startLBA & 0xff; /* lower byte of block (lba) */
                tsicp.m_wCylinder = (startLBA >> 8) & 0xffff; /* middle 2 bytes of block (lba) */
                tsicp.m_bDrivehead = IDE_DH_DEFAULT | /* set bits that must be on */
                        ((startLBA >> 24) & 0x0f) | /* lower nibble of byte 3 of block */
                        IDE_DH_DRIVE(nDriveIndex) |
                        IDE_DH_LBA;
            }
        }
    if(BootIdeIssueAtaCommand(uIoBase, ideWriteCommand, &tsicp))
    {
        printk("\n                      ide error %02X...\n", IoInputByte(IDE_REG_ERROR(uIoBase)));
        return 1;
    }

    while(remainingLen >= tsaHarddiskInfo[nDriveIndex].m_maxBlockTransfer){
        status = BootIdeWriteData(uIoBase, (u8 *)pbBuffer + bufferPtr, tsaHarddiskInfo[nDriveIndex].m_maxBlockTransfer * IDE_SECTOR_SIZE);   //Size in bytes here.
        bufferPtr += (tsaHarddiskInfo[nDriveIndex].m_maxBlockTransfer * IDE_SECTOR_SIZE);
        remainingLen -= tsaHarddiskInfo[nDriveIndex].m_maxBlockTransfer;
    }

    if(partialBlock && !status) //We have a last block to send and everything is good up to now
        status = BootIdeWriteData(uIoBase, (u8 *)pbBuffer + bufferPtr, partialBlock * IDE_SECTOR_SIZE);   //Size in bytes here.

    if(retry > 0)
        retry -= 1;
    if(status && retry){ //Status set to 1 or 2 means error. retry count must not be 0.
        printk("\n                 BootIdeWriteMultiple: write sector failed. %u retry left.", retry);

        //Retry (partial) block from the sector where it failed.
        status = BootIdeWriteMultiple(nDriveIndex, pbBuffer, startLBA, len, retry);      //Retry one more time.
    }
/*
    //Some drives requires a CACHE_FLUSH command being sent after each write command. We do it just to be sure.
    //So issue command. BSY has been cleared in BootIdeWriteData function
    tsicp = IDE_DEFAULT_COMMAND;
    //tsicp.m_bDrivehead = IDE_DH_DEFAULT | IDE_DH_HEAD(0) | IDE_DH_CHS | IDE_DH_DRIVE(nDriveIndex);
    if(tsaHarddiskInfo[nDriveIndex].m_dwCountSectorsTotal >= 0x10000000)        //LBA48 drive
        BootIdeIssueAtaCommand(uIoBase, IDE_CMD_CACHE_FLUSH_EXT, &tsicp);
    else
        BootIdeIssueAtaCommand(uIoBase, IDE_CMD_CACHE_FLUSH, &tsicp);
*/
    return status;
}

/* -------------------------------------------------------------------------------- */

///////////////////////////////////////////////
//      BootIdeBootSectorHddOrElTorito
//
//  Attempts to load boot code from Hdd or from CDROM/DVDROM
//   If HDD, loads MBR from Sector 0, if CDROM, uses El Torito to load default boot sector
//
// returns 0 if *pbaResult loaded with (512-byte/Hdd, 2048-byte/Cdrom) boot sector
//  otherwise nonzero return indicates error type
/*
int BootIdeBootSectorHddOrElTorito(int nDriveIndex, u8 * pbaResult)
{
    static const u8 baCheck11hFormat[] = {
            0x00,0x43,0x44,0x30,0x30,0x31,0x01,0x45,
            0x4C,0x20,0x54,0x4F,0x52,0x49,0x54,0x4F,
            0x20,0x53,0x50,0x45,0x43,0x49,0x46,0x49,
            0x43,0x41,0x54,0x49,0x4F,0x4E
    };
    int n;
    u32 * pdw;

    if(!tsaHarddiskInfo[nDriveIndex].m_fDriveExists) return 4;

    if(tsaHarddiskInfo[nDriveIndex].m_fAtapi) {
*/
/******   Numbnut's guide to El Torito CD Booting   ********

  Sector 11h of a bootable CDROM looks like this (11h is a magic number)
  The u32 starting at +47h is the sector index of the 'boot catalog'

00000000: 00 43 44 30 30 31 01 45 : 4C 20 54 4F 52 49 54 4F    .CD001.EL TORITO
00000010: 20 53 50 45 43 49 46 49 : 43 41 54 49 4F 4E 00 00     SPECIFICATION..
00000020: 00 00 00 00 00 00 00 00 : 00 00 00 00 00 00 00 00    ................
00000030: 00 00 00 00 00 00 00 00 : 00 00 00 00 00 00 00 00    ................
00000040: 00 00 00 00 00 00 00 13 : 00 00 00 00 00 00 00 00    ................
*/
/*
        if(BootIdeReadSector(nDriveIndex, &pbaResult[0], 0x11, 0, 2048)) {
            bprintf("Unable to get first sector\n");
            return 1;
        }

        for(n=0;n<sizeof(baCheck11hFormat);n++) {
            if(pbaResult[n]!=baCheck11hFormat[n]) {
                bprintf("Sector 11h not bootable format\n");
                return 2;
            }
        }

        pdw=(u32 *)&pbaResult[0x47];
*/
/*
At sector 13h (in this example only), the boot catalog:

00000000: 01 00 00 00 4D 69 63 72 : 6F 73 6F 66 74 20 43 6F    ....Microsoft Co
00000010: 72 70 6F 72 61 74 69 6F : 6E 00 00 00 4C 49 55 AA    rporation...LIU.
(<--- validation entry)
00000020: 88 00 00 00 00 00 04 00 : 25 01 00 00 00 00 00 00    ........%.......
(<-- initial/default entry - 88=bootable, 04 00 = 4 x (512-byte virtual sectors),
  = 1 x 2048-byte CDROM sector in boot, 25 01 00 00 = starts at sector 0x125)
*/
/*
        if(BootIdeReadSector(nDriveIndex, &pbaResult[0], *pdw, 0, 2048)) {
            bprintf("Unable to get boot catalog\n");
            return 3;
        }

        if((pbaResult[0]!=1) || (pbaResult[0x1e]!=0x55) || (pbaResult[0x1f]!=0xaa)) {
            bprintf("Boot catalog header corrupt\n");
            return 4;
        }

        if(pbaResult[0x20]!=0x88) {
            bprintf("Default boot catalog entry is not bootable\n");
            return 4;
        }

        pdw=(u32 *)&pbaResult[0x28];
*/
/*
And so at sector 0x125 (in this example only), we finally see the boot code

00000000: FA 33 C0 8E D0 BC 00 7C : FB 8C C8 8E D8 52 E8 00    .3.....|.....R..
00000010: 00 5E 81 EE 11 00 74 12 : 81 FE 00 7C 75 75 8C C8    .^....t....|uu..
00000020: 3D 00 00 75 7F EA 37 00 : C0 07 C6 06 AE 01 33 90    =..u..7.......3.
...
000007E0: 00 00 00 00 00 00 00 00 : 00 00 00 00 00 00 00 00    ................
000007F0: 00 00 00 00 00 00 00 00 : 00 00 00 00 00 00 55 AA    ..............U.
*/
/*
        if(BootIdeReadSector(nDriveIndex, &pbaResult[0], *pdw, 0, 2048)) {
            bprintf("Unable to get boot catalog\n");
            return 3;
        }

        if((pbaResult[0x7fe]!=0x55) || (pbaResult[0x7ff]!=0xaa)) {
            bprintf("Boot sector does not have boot signature!\n");
            return 4;
        }

        return 0; // success

    } else { // HDD boot

        if(BootIdeReadSector(nDriveIndex, &pbaResult[0], 0, 0, 512)) {
            bprintf("Unable to get MBR\n");
            return 3;
        }

        if((pbaResult[0x1fe]!=0x55) || (pbaResult[0x1ff]!=0xaa)) {
            bprintf("Boot sector does not have boot signature!\n");
            return 4;
        }

        return 0; // succes
    }
}
*/
    // these guys are used by grub
/* -------------------------------------------------------------------------------- */

int get_diskinfo (int drive, struct geometry *geometry)
{
    if(drive>1) return 1; // fail
    geometry->cylinders=tsaHarddiskInfo[drive].m_wCountCylinders;
    geometry->heads=tsaHarddiskInfo[drive].m_wCountHeads;
    geometry->sectors=tsaHarddiskInfo[drive].m_wCountSectorsPerTrack;
    geometry->total_sectors=tsaHarddiskInfo[drive].m_dwCountSectorsTotal;
    geometry->flags=0;
    return 0; // success
}

/* -------------------------------------------------------------------------------- */

int BootIdeSetTransferMode(int nIndexDrive, int nMode)
{
    tsIdeCommandParams tsicp = IDE_DEFAULT_COMMAND;
    unsigned int uIoBase = tsaHarddiskInfo[nIndexDrive].m_fwPortBase;
    
    tsicp.m_bDrivehead = IDE_DH_DEFAULT | IDE_DH_HEAD(0) | IDE_DH_CHS | IDE_DH_DRIVE(nIndexDrive);
    IoOutputByte(IDE_REG_DRIVEHEAD(uIoBase), tsicp.m_bDrivehead);
    
    IoOutputByte(0xff60+2, 0x62); // DMA possible for both drives

    IoOutputByte(IDE_REG_CONTROL(uIoBase), 0x08); // enable interrupt
    IoOutputByte(IDE_REG_FEATURE(uIoBase), 0x01); // enable DMA

    if(BootIdeWaitNotBusy(uIoBase)) {
            printk("\n       BootIdeSetTransferMode:Drive %d: Not Ready\n", nIndexDrive);
            return 1;
    }
    {
        int nReturn=0;
        tsicp.m_bCountSector = (u8)nMode;
        IoOutputByte(IDE_REG_FEATURE(uIoBase), 3); // set transfer mode subcmd
        nReturn=BootIdeIssueAtaCommand(uIoBase, IDE_CMD_SET_FEATURES, &tsicp);
        return nReturn;
    }
}

int BootIdeSetMultimodeSectors(u8 nIndexDrive, u8 nbSectors){
    tsIdeCommandParams tsicp = IDE_DEFAULT_COMMAND;
    unsigned int uIoBase = tsaHarddiskInfo[nIndexDrive].m_fwPortBase;
    u8 buffer[512];
    u16 *ptr = (u16 *)buffer;
    
    tsicp.m_bDrivehead = IDE_DH_DEFAULT | IDE_DH_HEAD(0) | IDE_DH_CHS | IDE_DH_DRIVE(0);
    IoOutputByte(IDE_REG_DRIVEHEAD(uIoBase), tsicp.m_bDrivehead);
    BootIdeWaitNotBusy(uIoBase);
    
    tsicp.m_bCountSector = nbSectors;           //Only relevant register(excluding Command register)
    if(BootIdeIssueAtaCommand(uIoBase, IDE_CMD_SET_MULTIPLE_MODE, &tsicp)){
        return 1;
    }

    BootIdeWaitNotBusy(uIoBase);
    
    if(BootIdeIssueAtaCommand(uIoBase, IDE_CMD_IDENTIFY, &tsicp)) {     //Check back our change
        //printk(" Drive %d: Not detected\n");
        return 1;
    }

    BootIdeWaitDataReady(uIoBase);

    if(BootIdeReadData(uIoBase, buffer, IDE_SECTOR_SIZE))
    {
        //printk("  %d: Drive not detected\n", nIndexDrive);
        return 1;
    }

    //check if it worked.
    if((nbSectors + 0x0100) == (ptr[59]&0x01FF)){	//bit8 should be 1 if MULTIPLE READ/WRITE commands are enabled.
        tsaHarddiskInfo[nIndexDrive].m_maxBlockTransfer = (ptr[59]&0x00FF);     //It worked
    }
    else{
        tsaHarddiskInfo[nIndexDrive].m_maxBlockTransfer = 0;       //Did not work.
    }
    return 0;                   //No error.
}

/*
//Set fastest PIO mode supported by HDD. Will probably be Mode4 since it was officially inserted in
//ATA-2 specs in 1996.
int BootIdeSetPIOMode(u8 nIndexDrive, u16 cycleTime){
    tsIdeCommandParams tsicp = IDE_DEFAULT_COMMAND;
    unsigned int uIoBase = tsaHarddiskInfo[nIndexDrive].m_fwPortBase;

    tsicp.m_bDrivehead = IDE_DH_DEFAULT | IDE_DH_HEAD(0) | IDE_DH_CHS | IDE_DH_DRIVE(0);
    IoOutputByte(IDE_REG_DRIVEHEAD(uIoBase), tsicp.m_bDrivehead);
    BootIdeWaitNotBusy(uIoBase);
    IoOutputByte(IDE_REG_FEATURE(uIoBase), 0x03);       //Set transfer mode sub-command

    //Depending on PIO cycle time value returned by IDENTIFY command, we select a PIO mode.
    //Can be from 0 to 4. One thing important is that bit3 must be set to 1(0x08).
    //Bits 2 to 0 select the PIO mode.
    if(cycleTime <= 120)        //Mode4
        tsicp.m_bCountSector = 0x0C;
    else if(cycleTime <= 180)   //Mode3
        tsicp.m_bCountSector = 0x0B;
    else if(cycleTime <= 240)   //Mode2
        tsicp.m_bCountSector = 0x0A;
    else if(cycleTime <= 383)   //Mode1
        tsicp.m_bCountSector = 0x09;
    else                        //Mode0
        tsicp.m_bCountSector = 0x08;

    if(BootIdeIssueAtaCommand(uIoBase, IDE_CMD_SET_FEATURES, &tsicp)){
        return 1;
    }

    BootIdeWaitNotBusy(uIoBase);

    return 0;                   //No error.
}
*/
