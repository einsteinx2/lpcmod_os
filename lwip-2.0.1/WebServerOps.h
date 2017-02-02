/*
 * WebServerOps.h
 *
 *  Created on: Oct 8, 2016
 *      Author: cromwelldev
 */

#ifndef LWIP_WEBSERVEROPS_H_
#define LWIP_WEBSERVEROPS_H_

#include <stdbool.h>

typedef enum
{
    NetworkState_Idle = 0U,
    NetworkState_Init = 1,
    NetworkState_DHCPStart = 2,
    NetworkState_ServerInit = 3,
    NetworkState_ServerRunning = 4,
    NetworkState_ServerShuttingDown = 5,
    NetworkState_Cleanup = 6
}NetworkState;

typedef enum
{
    WebServerOps_BIOSFlash = 0U,
    WebServerOps_EEPROMFlash = 1U,
    WebServerOps_HDD0Lock = 2U,
    WebServerOps_HDD1Lock = 3U,
    WebServerOps_NoOp = 0xFF
}WebServerOps;

void startNetFlash(WebServerOps flashType);
bool newPostProcessData(WebServerOps op, const unsigned char* buf, unsigned int size);
bool netflashPostProcess(void);
void run_lwip(void);

extern NetworkState currentNetworkState;
extern WebServerOps currentWebServerOp;

#endif /* LWIP_WEBSERVEROPS_H_ */
