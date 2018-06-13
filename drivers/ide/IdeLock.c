/*
 * IdeLock.c
 *
 *  Created on: Mar 26, 2018
 *      Author: cromwelldev
 */

#include "IdeDriver.h"
#include "IdeHelpers.h"
#include "BootHddKey.h"
#include "string.h"
#include "lib/LPCMod/BootLPCMod.h"

static int InternalSecurityChange(int driveId, const char *password, ide_command_t ide_cmd, bool forceMaster);

int IdeDriver_SecurityUnlock(int nDriveIndex, const char* szPassword, bool isMaster)
{
    return InternalSecurityChange(nDriveIndex, szPassword, IDE_CMD_SECURITY_UNLOCK, isMaster);
}

int IdeDriver_SetHDDPassword(int nDriveIndex, const char* szPassword)
{
    return InternalSecurityChange(nDriveIndex, szPassword, IDE_CMD_SECURITY_SET_PASSWORD, false);
}

int IdeDriver_SecurityDisable(int nDriveIndex, const char* szPassword, bool isMaster)
{
    return InternalSecurityChange(nDriveIndex, szPassword, IDE_CMD_SECURITY_DISABLE, isMaster);
}

/* -------------------------------------------------------------------------------- */
static int InternalSecurityChange(int driveId, const char *password, ide_command_t ide_cmd, bool forceMaster)
{
    //Todo: Check drive is in correct state for command desired.
    char ide_cmd_data[IDE_SECTOR_SIZE];
    unsigned short* wrdPtr = (unsigned short *)ide_cmd_data;
    tsIdeCommandParams tsicp = IDE_DEFAULT_COMMAND;
    const unsigned int uIoBase = tsaHarddiskInfo[driveId].m_fwPortBase;

    tsicp.m_bDrivehead = IDE_DH_DEFAULT | IDE_DH_DRIVE(driveId);

    if (ide_cmd == IDE_CMD_SECURITY_SET_PASSWORD) {
        //Cromwell locks drives in high security mode (NOT maximum security mode).
        //This means that even if you lose the normal (user) password, you can
        //unlock them again using the master password set below.
        //Master password is TEAMASSEMBLY (in ascii, NULL padded)
        const char *master_password="TEAMASSEMBLY";

        //We first lock the drive with the master password
        //Just in case we ever need to unlock it in an emergency
        memset(ide_cmd_data, 0x00, IDE_SECTOR_SIZE);
        //Set master password flag
        wrdPtr[0] = 0x01;

        memcpy(&ide_cmd_data[2], master_password, strlen(master_password));


        //Keep same Master Password Identifier.
        if(tsaHarddiskInfo[driveId].m_masterPassSupport != 0xFFFF)
        {
            memcpy(&ide_cmd_data[34], &(tsaHarddiskInfo[driveId].m_masterPassSupport),2);
            if(sendATACommandAndSendData(driveId, ide_cmd, NoStartLBA, ide_cmd_data, 1))
            {
                XBlastLogger(DEBUG_IDE_LOCK, DBG_LVL_FATAL, "Setting Master Password failed.");
                return 1;
            }
        }
    }

    memset(ide_cmd_data, 0x00, IDE_SECTOR_SIZE);

    //Password is only 20 bytes long - the rest is 0-padded.
    memcpy(&ide_cmd_data[2], password, 20);

    wrdPtr[0] = forceMaster ? 0x01 : 0x00;
    /* b8 or wrdPtr stays at '0' to ensure "High" security mode */

    if(sendATACommandAndSendData(driveId, ide_cmd, NoStartLBA, ide_cmd_data, 1))
    {
        return 1;
    }
    // check that we are unlocked
    BootIdeSendIdentifyDevice(driveId);
    //Success, hopefully.

    if(ide_cmd == IDE_CMD_SECURITY_SET_PASSWORD)
    {
        return IdeDriver_LockSecurityLevel_Disabled != IdeDriver_GetSecurityLevel(driveId) ? 0 : 1;
    }

    return IdeDriver_LockSecurityLevel_Disabled == IdeDriver_GetSecurityLevel(driveId) ? 0 : 1;;
}
