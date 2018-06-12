/*
 * IdeATAPI.c
 *
 *  Created on: Apr 8, 2018
 *      Author: cromwelldev
 */

#include "IdeDriverInternal.h"
#include "IdeHelpers.h"
#include "lib/time/timeManagement.h"
#include "string.h"
#include "stdio.h"
#include "video.h"
#include "boot.h"

static const char * const szaSenseKeys[] = {
    "No Sense", "Recovered Error", "Not Ready", "Medium Error",
    "Hardware Error", "Illegal request", "Unit Attention", "Data Protect",
    "Reserved 8", "Reserved 9", "Reserved 0xa", "Aborted Command",
    "Miscompare", "Reserved 0xf"
};

/////////////////////////////////////////////////
//  BootIdeAtapiModeSense
//
//  returns the ATAPI extra error info block
/*
int BootIdeAtapiModeSense(int nDriveIndex, unsigned char bCodePage, unsigned char * pba, int nLengthMaxReturn)
{
    unsigned uIoBase = tsaHarddiskInfo[nDriveIndex].m_fwPortBase;

    unsigned char ba[2048];
    int nReturn;

    if(!tsaHarddiskInfo[nDriveIndex].m_fDriveExists) return 4;

    memset(ba, 0, sizeof(ba));
    //memset(&ba[0], 0, 12);
    ba[0]=0x5a;
    ba[2]=bCodePage;
    ba[7]=(unsigned char)(sizeof(ba)>>8);
    ba[8]=(unsigned char)sizeof(ba);

    if(BootIdeIssueAtapiPacketCommandAndPacket(nDriveIndex, ba))
    {
//            unsigned char bStatus=IoInputByte(IDE_REG_STATUS(uIoBase)), bError=IoInputByte(IDE_REG_ERROR(uIoBase));
//            printk("  Drive %d: BootIdeAtapiAdditionalSenseCode FAILED, status=%02X, error=0x%02X, ASC unavailable\n", nDriveIndex, bStatus, bError);
            return 1;
    }

    nReturn=IoInputByte(IDE_REG_LBA_MID(uIoBase));
    nReturn |=IoInputByte(IDE_REG_LBA_HIGH(uIoBase))<<8;
    if(nReturn>nLengthMaxReturn) nReturn=nLengthMaxReturn;
    BootIdeReadData(uIoBase, pba, nReturn);

    return nReturn;
}
*
/* -------------------------------------------------------------------------------- */

/////////////////////////////////////////////////
//  IdeDriver_AtapiAdditionalSenseCode
//
//  returns the ATAPI extra error info block

int IdeDriver_AtapiAdditionalSenseCode(int nDriveIndex, unsigned char * pba, int nLengthMaxReturn)
{
    unsigned uIoBase = tsaHarddiskInfo[nDriveIndex].m_fwPortBase;

    unsigned char ba[2048];
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

    nReturn=IoInputByte(IDE_REG_LBA_MID(uIoBase));
    nReturn |=IoInputByte(IDE_REG_LBA_HIGH(uIoBase))<<8;
    if(nReturn>nLengthMaxReturn) nReturn=nLengthMaxReturn;
    BootIdeReadData(uIoBase, pba, nReturn);

    return nReturn;
}
/* -------------------------------------------------------------------------------- */

bool IdeDriver_AtapiReportFriendlyError(int nDriveIndex, char * szErrorReturn, int nMaxLengthError)
{
    unsigned char ba[2048];
    char szError[512];
    int nReturn;
    bool f=true;

    memset(ba, 0, sizeof(ba));
    nReturn=IdeDriver_AtapiAdditionalSenseCode(nDriveIndex, ba, sizeof(ba));
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

int Internal_ATAPIDataRead(int nDriveIndex, void * pbBuffer, unsigned int block, int n_bytes)
{
    unsigned char ba[12];
    tsIdeCommandParams tsicp = IDE_DEFAULT_COMMAND;
    int nReturn;
    int status;
    unsigned int uIoBase = tsaHarddiskInfo[nDriveIndex].m_fwPortBase;


    IoInputByte(IDE_REG_STATUS(uIoBase));
    if(IoInputByte(IDE_REG_STATUS(uIoBase)&1))
    {     // sticky error
        if(IoInputByte(IDE_REG_ERROR(uIoBase)&0x20))
        {     // needs attention
            if(IdeDriver_AtapiAdditionalSenseCode(nDriveIndex, ba, 2048)<12)
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
        return 1;
    }

    memset(ba, 0, sizeof(ba));
    ba[0]=0x28;
    ba[2]=block>>24;
    ba[3]=block>>16;
    ba[4]=block>>8;
    ba[5]=block;
    ba[7]=0;
    ba[8]=1;


    if(BootIdeIssueAtapiPacketCommandAndPacket(nDriveIndex, ba))
    {
        return 1;
    }

    nReturn=IoInputByte(IDE_REG_LBA_MID(uIoBase));
    nReturn |=IoInputByte(IDE_REG_LBA_HIGH(uIoBase))<<8;

    if(nReturn>2048) nReturn=2048;
    status = BootIdeReadData(uIoBase, pbBuffer, nReturn);
    if (status != 0)
    {
        while(1)
        {
            //XXX: Reduce wait delay length?
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
