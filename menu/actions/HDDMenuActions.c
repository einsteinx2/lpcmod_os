/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "HDDMenuActions.h"
#include "boot.h"
#include "BootIde.h"
#include "video.h"
#include "FatFSAccessor.h"
#include "TextMenu.h"
#include "lpcmod_v1.h"
#include "string.h"
#include "MenuActions.h"
#include "lib/time/timeManagement.h"
#include "lib/cromwell/cromString.h"
#include "FlashMenuActions.h"
#include "Gentoox.h"
#include "menu/misc/ConfirmDialog.h"
#include "WebServerOps.h"

static const char* formatCommonStr = "\n           Format ";

void AssertLockUnlock(void* customStructPtr)
{
    LockUnlockCommonParams* tempItemPtr = (LockUnlockCommonParams *)customStructPtr;
    unsigned char nIndexDrive = tempItemPtr->driveIndex;

    if((tsaHarddiskInfo[nIndexDrive].m_securitySettings & 0x0002) == 0x0002)     //Drive is already locked
    {
        UnlockHDD(nIndexDrive, 1, (unsigned char *)&eeprom, true);    //1 is for verbose
    }
    else
    {
        LockHDD(nIndexDrive, 1, (unsigned char *)&eeprom);    //1 is for verbose
    }

    if((tsaHarddiskInfo[nIndexDrive].m_securitySettings & 0x0002) == 0x0002)
    {
        sprintf(tempItemPtr->string1, "Unl");
        sprintf(tempItemPtr->string2, "Unl");

    }
    else
    {
        sprintf(tempItemPtr->string1, "L");
        sprintf(tempItemPtr->string2, "L");
    }
}

void AssertLockUnlockFromNetwork(void* customStructPtr)
{
    LockUnlockCommonParams* tempItemPtr = (LockUnlockCommonParams*)customStructPtr;
    unsigned char nIndexDrive = tempItemPtr->driveIndex;
    WebServerOps temp = WebServerOps_HDD1Lock;
    unsigned char *eepromPtr;

    if(nIndexDrive == 0)
    {
        temp = WebServerOps_HDD0Lock;
    }

    enableNetflash((void *)&temp);

    if((tsaHarddiskInfo[nIndexDrive].m_securitySettings & 0x0002) == 0x0002)
    {
        sprintf(tempItemPtr->string1, "Unl");
        sprintf(tempItemPtr->string2, "Unl");

    }
    else
    {
        sprintf(tempItemPtr->string1, "L");
        sprintf(tempItemPtr->string2, "L");
    }
}

bool LockHDD(int nIndexDrive, bool verbose, unsigned char* eepromPtr)
{
    unsigned char password[20];
    unsigned uIoBase = tsaHarddiskInfo[nIndexDrive].m_fwPortBase;
    int i;

    if(eepromPtr == NULL)
    {
        printk("\n\n\n\n\n");
        goto endExec;
    }

    if(verbose)
    {
        if(ConfirmDialog("Confirm Lock HDD?", 1))
        {
            return false;
        }
    }
    
    if(CalculateDrivePassword(nIndexDrive, password, eepromPtr))
    {
        printk("           Unable to calculate drive password - eeprom corrupt?");
        UIFooter();
        return false;
    }

    if(verbose)
    {
        printk("\n\n\n           XBlast OS locks drives with a master password of\n\n           \"\2TEAMASSEMBLY\2\"\n\n\n           Please remember this ");
        printk("as it could save your drive!\n\n");
        printk("           The normal password (user password) the drive is\n           being locked with is as follows:\n\n");
        printk("                              ");

        VIDEO_ATTR = 0xffef37;
        for(i = 0; i < 20; i++)
        {
            printk("\2%02x \2", password[i]);
            if((i + 1) % 5 == 0)
            {
                printk("\n\n                              ");
            }
        }    

        VIDEO_ATTR = 0xffffff;
        printk("\n           Locking drive");
        dots();
    }

    if(DriveSecurityChange(uIoBase, nIndexDrive, IDE_CMD_SECURITY_SET_PASSWORD, password))
    {
endExec:
        printk("\n           Locking drive failed");
        cromwellError();
        UIFooter();
        return false;
    }

    if(verbose)
    {
        printk("           Make a note of the password above.\n");
        UIFooter();
    }

    return true;
}

int UnlockHDD(int nIndexDrive, bool verbose, unsigned char* eepromPtr, bool internalEEPROM)
{
    unsigned char userPassword[21];
    int result = -1; //Start assuming not good.
    int i;

    if(eepromPtr == NULL)
    {
        printk("\n\n\n\n\n           Security disable failed. No EEPROM data supplied!");
        goto endExec;
    }


    if(tsaHarddiskInfo[nIndexDrive].m_securitySettings & 0x0010)            //Unlock attempt counter expired
    {
        printk("\n\n\n\n\n           \2Drive is now locked out.\n           \2Reboot system to reset HDD unlock capabilities.\n\n");
        UIFooter();
        return -1;
    }

    if(verbose)
    {
        if(ConfirmDialog("Confirm Unlock HDD?", true))
        {
            return false;
        }
    }

    CalculateDrivePassword(nIndexDrive, userPassword, eepromPtr);
    userPassword[20] = '\0';

    //HDD security has been disable (ie already have access?)
    if((tsaHarddiskInfo[nIndexDrive].m_securitySettings&0x0004)==0x0004)
    {
        //Do not try Master password unlock if eeprom pointer isn't pointing to internal eeprom image.
        if(internalEEPROM)
        {
            printk("\n\n           Something's wrong with the drive!\n           Jumping to Master Password Unlock sequence.");

            if(masterPasswordUnlockSequence(nIndexDrive))
            {
                result = 0;	//Sucess
                verbose = true;
            }
            else
            {
                result = -1;
            }

            goto endExec;
        }
        else
        {
            if(HDD_SECURITY_SendATACommand(nIndexDrive, IDE_CMD_SECURITY_UNLOCK, userPassword, false))
            {
                printk("\n\n           Unlock drive failed. Supplied EEPROM is not good!");
       	        result = -1;

       	        goto endExec;
            }
        }
    }
    
    
    if(HDD_SECURITY_SendATACommand(nIndexDrive, IDE_CMD_SECURITY_DISABLE, userPassword, false))
    {
        printk("\n\n           Unlock drive failed.");
        printk("\n           Password used was:");

        for(i = 0; i < strlen(userPassword); i++)
        {
            printk(" %02X", userPassword[i]);
        }
    	result = -1;

    	goto endExec;
    }
    else
    {
        result = 0;
    }
            
    if(result == 0)
    {
        if(verbose)
        {
            printk("\n\n\n           This drive is now unlocked.\n\n");
        }
    }

endExec:
    if(verbose)
    {
        UIFooter();
    }

    return result;
}

bool masterPasswordUnlockSequence(int nIndexDrive)
{
    unsigned char i;
    const char* MasterPasswordList[] =
    {
        "TEAMASSEMBLY",
        "XBOXSCENE",
        "Seagate                         ",
        "WDCWDCWDCWDCWDCWDCWDCWDCWDCWDCW"   //WDCWDCWDCWDCWDCWDCWDCWDCWDCWDCWD might also be valid. From personal experience WDCWDCWDCWDCWDCWDCWDCWDCWDCWDCW is more common.
    };
    printk("\n           Trying Master Password unlock.");

    for(i = 0; i < 4; i++)
    {
        if((tsaHarddiskInfo[nIndexDrive].m_securitySettings & 0x0010) == false)       //Drive is not locked out.
        {
            if(HDD_SECURITY_SendATACommand(nIndexDrive, IDE_CMD_SECURITY_UNLOCK, (char *)MasterPasswordList[i], true))
            {
                printk("\n           Master Password(%s) Unlock failed...", MasterPasswordList[i]);
            }
            else
            {
                HDD_SECURITY_SendATACommand(nIndexDrive, IDE_CMD_SECURITY_DISABLE, (char *)MasterPasswordList[i], true);
                printk("\n           Unlock Using Master Password %s successful.\n", MasterPasswordList[i]);

                return true;
            }
        }
        else
        {
            printk("\n           Drive is locked out. No further unlock attempts possible.\n           Power cycle console to reset HDD state.\n");
            break;
        }
    }

    printk("\n          Master Password Unlock failed.\n          No suitable password found.\n");
    return false;
}


void DisplayHDDPassword(void* customString)
{
    unsigned char nIndexDrive = ((LockUnlockCommonParams *)customString)->driveIndex;
    unsigned char password[20];
    int i;
    
    printk("\n\n\n           Calculating password");
    dots();

    if(CalculateDrivePassword(nIndexDrive,password, (unsigned char *)&eeprom))
    {
        cromwellError();
        wait_ms(2000);
        return;
    }
    
    cromwellSuccess();

    printk("           The normal password (user password) for this Xbox/Drive combination\n           is as follows:\n\n");
    printk("                              ");

    VIDEO_ATTR = 0xffef37;
    for(i = 0; i < 20; i++)
    {
        printk("\2%02x \2",password[i]);
        if ((i + 1) % 5 == 0)
        {
            printk("\n\n                              ");
        }
    }

    UIFooter();
}

void FormatCacheDrives(void* driveId)
{
    unsigned char nIndexDrive = *(unsigned char *)driveId;

    if(ConfirmDialog("Confirm format cache drives?", 1))
    {
        return;                                 //Cancel operation.
    }

    UiHeader("Format cache drives");
    if(fatxmkfs(nIndexDrive, Part_X))
    {
        cromwellError();
        printk("%s X: failed.\n", formatCommonStr);
    }
    else
    {
        cromwellSuccess();
        printk("%s X: success.\n", formatCommonStr);
    }

    if(fatxmkfs(nIndexDrive, Part_Y))
    {
        cromwellError();
        printk("%s Y: failed.\n", formatCommonStr);
    }
    else
    {
        cromwellSuccess();
        printk("%s Y: success.\n", formatCommonStr);
    }

    if(fatxmkfs(nIndexDrive, Part_Z))
    {
        cromwellError();
        printk("%s Z: failed.\n", formatCommonStr);
    }
    else
    {
        cromwellSuccess();
        printk("%s Z: success.\n", formatCommonStr);
    }

    UIFooter();
}

void FormatDriveC(void* driveId)
{
    unsigned char nIndexDrive = *(unsigned char *)driveId;

    if(ConfirmDialog("Confirm format C: drive?", 1))
    {
        return;                                 //Cancel operation.
    }
        
    UiHeader("Format C: drive");
    if(fatxmkfs(nIndexDrive, Part_C))
    {
        cromwellError();
        printk("%s C: failed.\n", formatCommonStr);
    }
    else
    {
        cromwellSuccess();
        printk("%s C: success.\n", formatCommonStr);
    }

    UIFooter();
}

void FormatDriveE(void* driveId)
{
    unsigned char nIndexDrive = *(unsigned char *)driveId;

    if(ConfirmDialog("Confirm format E: drive?", 1))
    {
        return;                                 //Cancel operation.
    }

    UiHeader("Format E: drive");
    if(fatxmkfs(nIndexDrive, Part_E))
    {
        cromwellError();
        printk("%s E: failed.\n", formatCommonStr);
    }
    else
    {
        cromwellSuccess();
        printk("%s E: success.\n", formatCommonStr);
    }
    UIFooter();
}

void DisplayHDDInfo(void* driveId)
{
    unsigned char nIndexDrive = *(unsigned char *)driveId;
    unsigned char i;
    XboxPartitionTable mbr;
    unsigned char clusterSize;
    unsigned int partSize;

    VIDEO_ATTR = 0xffffffff;

    printk("\n           Hard Disk Drive(%s)", nIndexDrive ? "slave":"master");
    printk("\n\n           Model : %s", tsaHarddiskInfo[nIndexDrive].m_szIdentityModelNumber);
    printk("\n           Serial : %s", tsaHarddiskInfo[nIndexDrive].m_szSerial);
    printk("\n           Firmware : %s", tsaHarddiskInfo[nIndexDrive].m_szFirmware);
    printk("\n           Capacity : %uGB", tsaHarddiskInfo[nIndexDrive].m_dwCountSectorsTotal / (2*1024*1024));     //In GB
    printk("\n           Sectors : %u ", tsaHarddiskInfo[nIndexDrive].m_dwCountSectorsTotal);
    printk("\n           # conductors : %u ", tsaHarddiskInfo[nIndexDrive].m_bCableConductors);
    printk("\n           Lock Status : %s ", ((tsaHarddiskInfo[nIndexDrive].m_securitySettings &0x0002)==0x0002) ? "Locked" : "Unlocked");
    printk("\n           FATX Formatted? : %s ", isFATXFormattedDrive(nIndexDrive) ? "Yes" : "No");

    if(0 == fatx_getmbr(nIndexDrive, &mbr))
    {
        for(i = 0; i < 7; i++)     //Print only info for C, E, F, G, X, Y and Z
        {
            if(mbr.TableEntries[i].Name[0] != ' ' && mbr.TableEntries[i].LBAStart != 0)    //Valid partition entry only
            {
                printk("\n                 %s", mbr.TableEntries[i].Name);
                printk("\n                     Active: %s", mbr.TableEntries[i].Flags == FATX_PE_PARTFLAGS_IN_USE ? "Yes" : "No");

                if(mbr.TableEntries[i].LBASize >= LBASIZE_512GB)           //Need 64K clusters
                {
                    clusterSize = 64;                                      //Clustersize in number of 512-byte sectors
                }
                else if(mbr.TableEntries[i].LBASize >= LBASIZE_256GB)
                {
                    clusterSize = 32;
                }
                else if(mbr.TableEntries[i].LBASize >= 1)
                {
                    clusterSize = 16;
                }
                else
                {
                    clusterSize = 0;
                }
                partSize = mbr.TableEntries[i].LBASize / 2048;      //in MB
                printk("    Size: %uMB   Cluster: %uKB", partSize, clusterSize);
            }
        }
    }

    UIFooter();
}

void FormatDriveFG(void* driveId)
{
    unsigned char nDriveIndex = (*(unsigned char *)driveId) & 0x0f;
    unsigned char formatOption = (*(unsigned char *)driveId) & 0xf0;
    unsigned char buffer[100];
    XboxDiskLayout selectedLayout = XboxDiskLayout_Base;

    switch(formatOption)
    {
        case F_GEQUAL:                                  //Split amount of sectors evenly on 2 partitions
            sprintf(buffer, "%s", "Confirm format:\n\2F:, G: Split evenly?");
            selectedLayout = XboxDiskLayout_FGSplit;
            break;
        case FMAX_G:            //F = LBASIZE_1024GB - 1 and G: takes the rest
            sprintf(buffer, "%s", "Confirm format:\n\2Max F:, G: takes the rest?");
            selectedLayout = XboxDiskLayout_FMaxGRest;
            break;
        case F137_G:            //F = LBASIZE_137GB and G takes the rest
            sprintf(buffer, "%s", "Confirm format:\n\2F: = 120GB, G: takes the rest?");
            selectedLayout = XboxDiskLayout_F120GRest;
            break;
        case F_NOG:             //F < LBASIZE_1024GB - 1.
            sprintf(buffer, "%s", "Confirm format:\n\2F: take all, no G:?");
            selectedLayout = XboxDiskLayout_FOnly;
            break;
        default:
            return;
            break;
    }


    if(ConfirmDialog(buffer, 1) == false)
    {
        if(fdisk(nDriveIndex, selectedLayout))
        {
            UiHeader("Error!!");
            //fail
            cromwellError();
            printk("\n           Partitionning failed.");
            UIFooter();
        }
        else
        {
            UiHeader("Format F: drive");
            if(fatxmkfs(nDriveIndex, Part_F))
            {
                cromwellError();
                printk("%sfailed.", formatCommonStr);
            }
            else
            {
                cromwellSuccess();
                printk("%ssuccess.", formatCommonStr);
            }
            UIFooter();

            if(formatOption != F_NOG)
            {
                UiHeader("Format G: drive");
                if(fatxmkfs(nDriveIndex, Part_G))
                {
                    cromwellError();
                    printk("%sfailed.", formatCommonStr);
                }
                else
                {
                    cromwellSuccess();
                    printk("%ssuccess.", formatCommonStr);
                }
                UIFooter();
            }
        }
    }
}

void AssertSMARTEnableDisable(void* customString)
{
    LockUnlockCommonParams* tempItemPtr = (LockUnlockCommonParams *)customString;
    unsigned char nIndexDrive = tempItemPtr->driveIndex;

    if(tsaHarddiskInfo[nIndexDrive].m_fSMARTEnabled)        //Drive is already locked
    {
        driveToggleSMARTFeature(nIndexDrive, 0xD9);          //0xD9 is subcommand for disabling SMART.
    }
    else
    {
        driveToggleSMARTFeature(nIndexDrive, 0xD8);          //0xD8 is subcommand for enabling SMART.
    }
    if(tsaHarddiskInfo[nIndexDrive].m_fSMARTEnabled)
    {
        sprintf(tempItemPtr->string1, "%s", "Disable");
    }
    else
    {
        sprintf(tempItemPtr->string1, "%s", "Enable");
    }
}

void CheckSMARTRETURNSTATUS(void* customString)
{
    unsigned char nIndexDrive = ((LockUnlockCommonParams *)customString)->driveIndex;
    int pollReturn;

    UiHeader("Read S.M.A.R.T. status");

    VIDEO_ATTR = 0xffffffff;

    if(tsaHarddiskInfo[nIndexDrive].m_fSMARTEnabled)
    {
        pollReturn = driveSMARTRETURNSTATUS(nIndexDrive);
        printk("\n\n\n\1          S.M.A.R.T. return ");

        if(pollReturn == 0)
        {
            printk("drive is fine!");
        }
        else if(pollReturn == 1)
        {
            printk("drive exceeded threshold!\n\1           Please test drive!");
        }
        else
        {
            printk("unknown S.M.A.R.T. status...");
        }
    }
    else
    {
        printk("\n\1          S.M.A.R.T. not enabled.\n\1          Please enable S.M.A.R.T. to use this feature.");
    }

    UIFooter();
}
