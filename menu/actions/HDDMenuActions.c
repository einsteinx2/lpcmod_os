/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "HDDMenuActions.h"

void AssertLockUnlock(void *driveId){
    int nIndexDrive = *(int *)driveId;
    if((tsaHarddiskInfo[nIndexDrive].m_securitySettings &0x0004)==0x0004) {       //Drive is already locked
        UnlockHDD(nIndexDrive);
    }
    else {
        LockHDD(nIndexDrive);
    }
}

void LockHDD(int nIndexDrive) {
    u8 password[20];
    unsigned uIoBase = tsaHarddiskInfo[nIndexDrive].m_fwPortBase;
    int i;

    if (!Confirm("Confirm locking", "Yes", "No", 0)) return;    
    
    if (CalculateDrivePassword(nIndexDrive,password)) {
        printk("           Unable to calculate drive password - eeprom corrupt?");
        return;
    }
    printk("\n\n\n\n\n           XBlast OS locks drives with a master password of\n\n           \"\2TEAMASSEMBLY\2\"\n\n\n           Please remember this ");
    printk("as it could save your drive!\n\n");
    printk("           The normal password (user password) the drive is\n           being locked with is as follows:\n\n");
    printk("                              ");
    VIDEO_ATTR=0xffef37;
    for (i=0; i<20; i++) {
        printk("\2%02x \2",password[i]);
        if ((i+1)%5 == 0) {
            printk("\n\n                              ");
        }
    }    
    VIDEO_ATTR=0xffffff;
    printk("\n           Locking drive");
    dots();
    if (DriveSecurityChange(uIoBase, nIndexDrive, IDE_CMD_SECURITY_SET_PASSWORD, password)) {
        cromwellError();
        while(1);
    }
    cromwellSuccess();
    printk("           Make a note of the password above.\n");
    printk("           Press Button A to continue");

    while ((risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_A) != 1)) wait_ms(10);
}

void UnlockHDD(int nIndexDrive) {
    u8 password[20];
    unsigned uIoBase = tsaHarddiskInfo[nIndexDrive].m_fwPortBase;
    
    if (!Confirm("Confirm unlocking", "Yes", "No", 0)) return;    
    
    if (CalculateDrivePassword(nIndexDrive,password)) {
        printk("           Unable to calculate drive password - eeprom corrupt?  Aborting\n");
        return;
    }
    if (DriveSecurityChange(uIoBase, nIndexDrive, IDE_CMD_SECURITY_DISABLE, password)) {
        printk("           Failed!");
    }
    printk("\n\n\n\n\n           \2This drive is now unlocked.\n\n");
    printk("           \2Press Button A to continue");

    while ((risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_A) != 1)) wait_ms(10);
}


void DisplayHDDPassword(void *driveId) {
    int nIndexDrive = *(int *)driveId;
    u8 password[20];
    int i;
    
    printk("\n\n\n\n\n           Calculating password");
    dots();
    if (CalculateDrivePassword(nIndexDrive,password)) {
        cromwellError();
        wait_ms(2000);
        return;
    }
    
    cromwellSuccess();

    printk("           The normal password (user password) for this drive is as follows:\n\n");
    printk("                              ");
    VIDEO_ATTR=0xffef37;
    for (i=0; i<20; i++) {
        printk("\2%02x \2",password[i]);
        if ((i+1)%5 == 0) {
            printk("\n\n                              ");
        }
    }    
    VIDEO_ATTR=0xffffff;
    printk("\n\n           Press Button A to continue.");

    while ((risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_A) != 1)) wait_ms(10);
}

void FormatCacheDrives(void *driveId){
    u8 buffer[512], headerBuf[0x1000], chainmapBuf[512];
    u32 counter;
    u32 whichpartition;
    PARTITIONHEADER *header;

    int nIndexDrive = *(int *)driveId;

    if(ConfirmDialog("              Confirm format cache drives?", 1))
        return;                                 //Cancel operation.

    FATXCheckAndSetBRFR(nIndexDrive);

    memset(headerBuf,0xff,0x1000);              //First sector(and only one used) of the Partition header area.
    header = (PARTITIONHEADER *)headerBuf;
    header->magic = FATX_PARTITION_MAGIC;       //Whooo, magic!
    header->volumeID = IoInputDword(0x8008);    //Goes with the HDD.
    header->clusterSize = 0x20;                 //16KB cluster, so 32 clusters per sector.
    header->nbFAT = 1;                          //Always 1.
    header->unknown = 0;                        //Always 0.
    memset(header->unused,0xff,4078);           //Fill unused area with 0xff.
    //Partition header is ready. It's the same for all 3 cache drives.
    //It's not necessary to fill unused area up to 0x1000 because we'll only write the first 512 bytes of headerBuf
    //onto the HDD. Let's do it for the exercise OK? A few wasted cycles isn't going to hurt anybody.

    memset(chainmapBuf,0x0,512);                //First sector of the Cluster chain map area.
    chainmapBuf[0]=0xf8;                        //First cluster is 0xFFFFFFF8 in 4 byte mode cluster.
    chainmapBuf[1]=0xff;
    chainmapBuf[2]=0xff;
    chainmapBuf[3]=0xff;

    memset(buffer,0xff,512);                    //Killer buffer.

    //Cycle through all 3 cache partitions.
    for(whichpartition = SECTOR_CACHE1; whichpartition < SECTOR_CACHE3; whichpartition += (SECTOR_CACHE2 - SECTOR_CACHE1)){
        // Starting (from 0 to 512*8 = 0x1000). Erasing Partition header data.
        //4KB so 8*512 bytes sectors.
        for (counter=whichpartition;counter<(whichpartition+8); counter++) {
            BootIdeWriteSector(nIndexDrive,buffer,counter);
        }
        //Write Partition info on first sector. lest seven sectors of the first 0x1000 are already 0xff anyway.
        BootIdeWriteSector(nIndexDrive,headerBuf,whichpartition);   //Partition header write.

        // Cluster chain map area (from 512*8 = 0x1000 to 512*200 = 0x19000)
        memset(buffer,0x0,512); //wipe. Unused cluster == 0
        for (counter=(whichpartition+8);counter<(whichpartition+200); counter++) {
            BootIdeWriteSector(nIndexDrive,buffer,counter);
        }
        //Write initial cluster chain map.
        BootIdeWriteSector(nIndexDrive,chainmapBuf,whichpartition+8);   //Initial Cluster chain map write.

        // Root Dir (from 512*200 = 0x19000 to 0x1d000 = 512*232)
        memset(buffer,0xff,512);
        for (counter=(whichpartition+200);counter<(whichpartition+200+(32*10)); counter++) {
            BootIdeWriteSector(nIndexDrive,buffer,counter);
        }
    }
    HDDMenuHeader("Cache drives formatted");
    HDDMenuFooter();
}

void FormatDriveC(void *driveId){
    u8 buffer[512], headerBuf[0x1000], chainmapBuf[512];
    u32 counter;
    PARTITIONHEADER *header;

    int nIndexDrive = *(int *)driveId;

    if(ConfirmDialog("           Confirm format C: drive?", 1))
        return;                                 //Cancel operation.

    FATXCheckAndSetBRFR(nIndexDrive);

    memset(headerBuf,0xff,0x1000);              //First sector(and only one used) of the Partition header area.
    header = (PARTITIONHEADER *)headerBuf;
    header->magic = FATX_PARTITION_MAGIC;       //Whooo, magic!
    header->volumeID = IoInputDword(0x8008);    //Goes with the HDD.
    header->clusterSize = 0x20;                 //16KB cluster, so 32 clusters per sector.
    header->nbFAT = 1;                          //Always 1.
    header->unknown = 0;                        //Always 0.
    memset(header->unused,0xff,4078);           //Fill unused area with 0xff.
    //Partition header is ready. It's the same for all 3 cache drives.
    //It's not necessary to fill unused area up to 0x1000 because we'll only write the first 512 bytes of headerBuf
    //onto the HDD. Let's do it for the exercise OK? A few wasted cycles isn't going to hurt anybody.

    memset(chainmapBuf,0x0,512);                //First sector of the Cluster chain map area.
    chainmapBuf[0]=0xf8;                        //First cluster is 0xFFFFFFF8 in 4 byte mode cluster.
    chainmapBuf[1]=0xff;
    chainmapBuf[2]=0xff;
    chainmapBuf[3]=0xff;

    memset(buffer,0xff,512);                    //Killer buffer.

    // Starting (from 0 to 512*8 = 0x1000). Erasing Partition header data.
    //4KB so 8*512 bytes sectors.
    for (counter=SECTOR_SYSTEM;counter<(SECTOR_SYSTEM+8); counter++) {
        BootIdeWriteSector(nIndexDrive,buffer,counter);
    }
    //Write Partition info on first sector. lest seven sectors of the first 0x1000 are already 0xff anyway.
    BootIdeWriteSector(nIndexDrive,headerBuf,SECTOR_SYSTEM);   //Partition header write.

    // Cluster chain map area (from 512*8 = 0x1000 to 512*136 = 0x11000)
    memset(buffer,0x0,512); //wipe. Unused cluster == 0
    for (counter=(SECTOR_SYSTEM+8);counter<(SECTOR_SYSTEM+136); counter++) {
        BootIdeWriteSector(nIndexDrive,buffer,counter);
    }
    //Write initial cluster chain map.
    BootIdeWriteSector(nIndexDrive,chainmapBuf,SECTOR_SYSTEM+8);   //Initial Cluster chain map write.

    // Root Dir (from 512*136 = 0x11000 )
    memset(buffer,0xff,512);
    for (counter=(SECTOR_SYSTEM+136);counter<(SECTOR_SYSTEM+136+(32*10)); counter++) {
        BootIdeWriteSector(nIndexDrive,buffer,counter);
    }

    HDDMenuHeader("C: drive formatted");
    HDDMenuFooter();
}

void FormatDriveE(void *driveId){
    u8 buffer[512], headerBuf[0x1000], chainmapBuf[512];
    u32 counter;
    PARTITIONHEADER *header;

    int nIndexDrive = *(int *)driveId;

    if(ConfirmDialog("           Confirm format E: drive?", 1))
        return;                                 //Cancel operation.

    FATXCheckAndSetBRFR(nIndexDrive);

    memset(headerBuf,0xff,0x1000);              //First sector(and only one used) of the Partition header area.
    header = (PARTITIONHEADER *)headerBuf;
    header->magic = FATX_PARTITION_MAGIC;       //Whooo, magic!
    header->volumeID = IoInputDword(0x8008);    //Goes with the HDD.
    header->clusterSize = 0x20;                 //16KB cluster, so 32 clusters per sector.
    header->nbFAT = 1;                          //Always 1.
    header->unknown = 0;                        //Always 0.
    memset(header->unused,0xff,4078);           //Fill unused area with 0xff.
    //Partition header is ready. It's the same for all 3 cache drives.
    //It's not necessary to fill unused area up to 0x1000 because we'll only write the first 512 bytes of headerBuf
    //onto the HDD. Let's do it for the exercise OK? A few wasted cycles isn't going to hurt anybody.

    memset(chainmapBuf,0x0,512);                //First sector of the Cluster chain map area.
    memset(chainmapBuf,0xff,4*7);               //We'll use clusters for base folders.
    chainmapBuf[0]=0xf8;                        //First cluster is 0xFFFFFFF8 in 4 byte mode cluster.

    memset(buffer,0xff,512);                    //Killer buffer.

    // Starting (from 0 to 512*8 = 0x1000). Erasing Partition header data.
    //4KB so 8*512 bytes sectors.
    for (counter=SECTOR_STORE;counter<(SECTOR_STORE+8); counter++) {
        BootIdeWriteSector(nIndexDrive,buffer,counter);
    }
    //Write Partition info on first sector. lest seven sectors of the first 0x1000 are already 0xff anyway.
    BootIdeWriteSector(nIndexDrive,headerBuf,SECTOR_STORE);   //Partition header write.

    // Cluster chain map area (from 512*8 = 0x1000 to 512*2456 = 0x133000)
    memset(buffer,0x0,512); //wipe. Unused cluster == 0
    for (counter=(SECTOR_STORE+8);counter<(SECTOR_STORE+2440); counter++) {
        BootIdeWriteSector(nIndexDrive,buffer,counter);
    }
    //Write initial cluster chain map.
    BootIdeWriteSector(nIndexDrive,chainmapBuf,SECTOR_STORE+8);   //Initial Cluster chain map write.

    // Marked as used from 2440 to 2456.
    memset(buffer,0xff,512);
    for (counter=(SECTOR_STORE+2440);counter<(SECTOR_STORE+2456); counter++) {
        BootIdeWriteSector(nIndexDrive,buffer,counter);
    }

    // Root Dir (from 512*2456 = 0x133000 to 0x1d000 = 512*232)
    // 10 cluster formatted.
    for (counter=(SECTOR_STORE+2456);counter<(SECTOR_STORE+2456+(32*10)); counter++) {
        BootIdeWriteSector(nIndexDrive,buffer,counter);
    }

    memset(buffer,0xff,512);
    // TDATA Dir on Cluster 2
    FATXCreateDirectoryEntry(buffer,"TDATA",0,2);
    // UDATA Dir on Cluster 3
    FATXCreateDirectoryEntry(buffer,"UDATA",1,3);
    // CACHE Dir on Cluster 6
    FATXCreateDirectoryEntry(buffer,"CACHE",2,6);
    BootIdeWriteSector(nIndexDrive,buffer,SECTOR_STORE+2456);   // Write Cluster 1.

    memset(buffer,0xff,512);
    // FFFE0000 Dir on Cluster 4
    FATXCreateDirectoryEntry(buffer,"FFFE0000",0,4);
    BootIdeWriteSector(nIndexDrive,buffer,SECTOR_STORE+2456+32);   // Write Cluster 2.

    memset(buffer,0xff,512);
    // Music Dir on Cluster 5
    FATXCreateDirectoryEntry(buffer,"Music",0,5);
    BootIdeWriteSector(nIndexDrive,buffer,SECTOR_STORE+2456+32+32+32);   // Write Cluster 4.


    HDDMenuHeader("E: drive formatted");
    HDDMenuFooter();
}

void HDDMenuFooter(void) {
    VIDEO_ATTR=0xffc8c8c8;
    printk("\n\n           Press Button 'A' to continue.");
    while ((risefall_xpad_BUTTON(TRIGGER_XPAD_KEY_A) != 1)) wait_ms(10);
}

void HDDMenuHeader(char *title) {
    printk("\n\n\n\n\n           ");
    VIDEO_ATTR=0xffffef37;
    printk("\2%s\2\n\n\n\n           ", title);
    VIDEO_ATTR=0xffc8c8c8;
}
