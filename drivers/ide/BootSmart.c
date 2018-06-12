/*
 * BootSmart.c
 *
 *  Created on: Mar 26, 2018
 *      Author: cromwelldev
 */

#include "IdeDriver.h"
#include "IdeHelpers.h"
#include "lib/time/timeManagement.h"
#include "lib/cromwell/cromString.h"
#include "boot.h"

/* S.M.A.R.T. commands requires to have this constant placed at bits 23:8 in the LBA start value */
#define SMART_CMD_LBA_FIELD  0xC24F << 8

#define DISABLE_SMART_SUB_CMD 0xD9
#define ENABLE_SMART_SUB_CMD 0xD8
#define RETURN_STATUS_SMART_SUB_CMD 0xDA

//Send SMART commands. Specify input command by smart_cmd.
//Return true if error.
bool IdeDriver_ToggleSMARTFeature(int nDriveIndex, bool enable)
{
    unsigned int lbaField = SMART_CMD_LBA_FIELD;
    sendControlATACommand(nDriveIndex, IDE_CMD_SMART, lbaField, enable ? ENABLE_SMART_SUB_CMD : DISABLE_SMART_SUB_CMD, NoSectorCount);
    BootIdeSendIdentifyDevice(nDriveIndex);

    //check if it worked.
    return IdeDriver_DeviceSMARTEnabled(nDriveIndex);
}

int IdeDriver_SMARTReturnStatus(int nDriveIndex){
    int result = -1;    //Start assuming error.
    unsigned short w;
    unsigned char error;

    unsigned int lbaField = SMART_CMD_LBA_FIELD + 0x1; // + 0x1 for Summary SMART error log
    sendControlATACommand(nDriveIndex, IDE_CMD_SMART, lbaField, RETURN_STATUS_SMART_SUB_CMD, NoSectorCount);

    wait_us_blocking(1);

    w=IoInputByte(IDE_REG_LBA_MID(tsaHarddiskInfo[nDriveIndex].m_fwPortBase));
    w|=(IoInputByte(IDE_REG_LBA_HIGH(tsaHarddiskInfo[nDriveIndex].m_fwPortBase)))<<8;

    error=IoInputByte(IDE_REG_STATUS(tsaHarddiskInfo[nDriveIndex].m_fwPortBase));
    if(error&1) { // error
        return -1;
    }

    if(w == 0xC24F)     //Everything is fine
        result = 0;
    else if(w == 0x2CF4)        //Threshold exceeded condition.
        result = 1;
    else{
        printk("\n       Weird SMART status : %04X", w);
        result = -1;            //wtf
    }

    return result;
}
