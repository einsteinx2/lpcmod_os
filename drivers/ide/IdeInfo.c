/*
 * IdeInfo.c
 *
 *  Created on: Apr 8, 2018
 *      Author: cromwelldev
 */

#include "IdeDriver.h"
#include "IdeHelpers.h"

const char* const IdeDriver_GetModelNumber(int nDriveIndex)
{
    return tsaHarddiskInfo[nDriveIndex].m_szIdentityModelNumber;
}

const char* const IdeDriver_GetSerialNumber(int nDriveIndex)
{
    return tsaHarddiskInfo[nDriveIndex].m_szSerial;
}

const char* const IdeDriver_GetFirmwareVersion(int nDriveIndex)
{
    return tsaHarddiskInfo[nDriveIndex].m_szFirmware;
}

int IdeDriver_GetCableConductorCount(int nDriveIndex)
{
    return tsaHarddiskInfo[nDriveIndex].m_bCableConductors;
}

unsigned long long IdeDriver_GetSectorCount(int nDriveIndex)
{
    return tsaHarddiskInfo[nDriveIndex].m_dwCountSectorsTotal;
}

int IdeDriver_GetSectorSize(int nDriveIndex)
{
    // TODO: retrieve from IDENTIFY cmd
    return IDE_SECTOR_SIZE;
}

int IdeDriver_DeviceConnected(int nDriveIndex)
{
    return tsaHarddiskInfo[nDriveIndex].m_fDriveExists;
}

int IdeDriver_DeviceIsATAPI(int nDriveIndex)
{
    return tsaHarddiskInfo[nDriveIndex].m_fAtapi;
}

int IdeDriver_DeviceSecuritySupport(int nDriveIndex)
{
    return tsaHarddiskInfo[nDriveIndex].m_bSecurityFeaturesSupport;
}

int IdeDriver_DeviceIsLocked(int nDriveIndex)
{
    return tsaHarddiskInfo[nDriveIndex].m_bSecurityLocked;
}

IdeDriver_LockSecurityLevel IdeDriver_GetSecurityLevel(int nDriveIndex)
{
    if(tsaHarddiskInfo[nDriveIndex].m_bSecurityEnabled)
    {
        if(tsaHarddiskInfo[nDriveIndex].m_currentSecurityLevel)
        {
            return IdeDriver_LockSecurityLevel_Maximum;
        }
        return IdeDriver_LockSecurityLevel_High;
    }

    return IdeDriver_LockSecurityLevel_Disabled;
}

int IdeDriver_DeviceUnlockCounterExpired(int nDriveIndex)
{
    return tsaHarddiskInfo[nDriveIndex].m_bSecurityCountExpired;
}

int IdeDriver_DeviceSMARTCapable(int nDriveIndex)
{
    return tsaHarddiskInfo[nDriveIndex].m_fHasSMARTcapabilities;
}

int IdeDriver_DeviceSMARTEnabled(int nDriveIndex)
{
    if(IdeDriver_DeviceSMARTCapable(nDriveIndex))
    {
        return tsaHarddiskInfo[nDriveIndex].m_fSMARTEnabled;
    }

    return 0;
}
