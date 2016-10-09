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
    WebServerOps_EEPROMFlash,
    WebServerOps_HDD0Lock,
    WebServerOps_HDD1Lock
}WebServerOps;

extern WebServerOps currentWebServerOp;

#endif /* LWIP_WEBSERVEROPS_H_ */
