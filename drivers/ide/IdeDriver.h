/*
 * IdeDriver.h
 *
 *  Created on: Jun 6, 2018
 *      Author: cromwelldev
 */

#ifndef DRIVERS_IDE_IDEDRIVER_H_
#define DRIVERS_IDE_IDEDRIVER_H_

#include <stdbool.h>

/*********************************************/
/* Init */
/*********************************************/
void IdeDriver_Init(void);

/*********************************************/
/* DeviceInfo */
/*********************************************/
const char* const IdeDriver_GetModelNumber(int nDriveIndex);
const char* const IdeDriver_GetSerialNumber(int nDriveIndex);
const char* const IdeDriver_GetFirmwareVersion(int nDriveIndex);
int IdeDriver_GetCableConductorCount(int nDriveIndex);
typedef enum
{
    IdeDriver_LockSecurityLevel_Disabled,
    IdeDriver_LockSecurityLevel_High,
    IdeDriver_LockSecurityLevel_Maximum
}IdeDriver_LockSecurityLevel;
unsigned long long IdeDriver_GetSectorCount(int nDriveIndex);
int IdeDriver_GetSectorSize(int nDriveIndex);
int IdeDriver_DeviceConnected(int nDriveIndex);
int IdeDriver_DeviceIsATAPI(int nDriveIndex);
int IdeDriver_DeviceSecuritySupport(int nDriveIndex);
int IdeDriver_DeviceIsLocked(int nDriveIndex);
IdeDriver_LockSecurityLevel IdeDriver_GetSecurityLevel(int nDriveIndex);
int IdeDriver_DeviceUnlockCounterExpired(int nDriveIndex);
int IdeDriver_DeviceSMARTCapable(int nDriveIndex);
int IdeDriver_DeviceSMARTEnabled(int nDriveIndex);


/*********************************************/
/* ATALock */
/*********************************************/
int IdeDriver_SecurityUnlock(int nDriveIndex, const char* szPassword, bool isMaster); /* This is the one to give data access */
int IdeDriver_SetHDDPassword(int nDriveIndex, const char* szPassword);  /* Commonly reffered on the scene as lock HDD */
int IdeDriver_SecurityDisable(int nDriveIndex, const char* szPassword, bool isMaster); /* Commonly reffered on the scene as unlock HDD */


/*********************************************/
/* S.M.A.R.T. */
/*********************************************/
bool IdeDriver_ToggleSMARTFeature(int nDriveIndex, bool enable);
int IdeDriver_SMARTReturnStatus(int nDriveIndex);


/*********************************************/
/* Data Access */
/*********************************************/
int IdeDriver_Read(int nDriveIndex, void * pbBuffer, unsigned int startSector, int byteCount) ;
int IdeDriver_Write(int nDriveIndex, const void * pbBuffer, unsigned int startSector, int byteCount);
int IdeDriver_FlushCache(int nDriveIndex);


/*********************************************/
/* ATAPI */
/*********************************************/
int IdeDriver_AtapiAdditionalSenseCode(int nDriveIndex, unsigned char * pba, int nLengthMaxReturn);
bool IdeDriver_AtapiReportFriendlyError(int nDriveIndex, char * szErrorReturn, int nMaxLengthError);

#endif /* DRIVERS_IDE_IDEDRIVER_H_ */
