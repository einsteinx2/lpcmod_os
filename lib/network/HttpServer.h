/*
 * HttpServer.h
 *
 *  Created on: May 18, 2018
 *      Author: cromwelldev
 */

#ifndef LIB_NETWORK_HTTPSERVER_H_
#define LIB_NETWORK_HTTPSERVER_H_

#include "stdbool.h"

typedef enum
{
    WebServerOps_BIOSFlash = 0U,
    WebServerOps_EEPROMFlash = 1U,
    WebServerOps_HDD0Lock = 2U,
    WebServerOps_HDD1Lock = 3U,
    WebServerOps_NoOp = 0xFF
}WebServerOps;

void NetworkManager_httpdInit(WebServerOps flashType);
void NetworkManager_httpdHalt(void);

bool netflashPostProcess(void);
bool newPostProcessData(WebServerOps op, const unsigned char* buf, unsigned int size);

WebServerOps getWebServerOps(void);

#endif /* LIB_NETWORK_HTTPSERVER_H_ */
