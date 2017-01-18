/*
 * WebServerOps.h
 *
 *  Created on: Oct 8, 2016
 *      Author: cromwelldev
 */

#ifndef LWIP_WEBSERVEROPS_H_
#define LWIP_WEBSERVEROPS_H_

typedef enum
{
    WebServerOps_BIOSFlash = 0U,
    WebServerOps_EEPROMFlash = 1U,
    WebServerOps_HDD0Lock = 2U,
    WebServerOps_HDD1Lock = 3U
}WebServerOps;

extern WebServerOps currentWebServerOp;

#endif /* LWIP_WEBSERVEROPS_H_ */
