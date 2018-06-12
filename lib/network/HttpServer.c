/*
 * HttpServer.c
 *
 *  Created on: May 18, 2018
 *      Author: cromwelldev
 */

#include "HttpServer.h"
#include "stdbool.h"
#include "stddef.h"
#include "stdlib.h"
#include "FlashUi.h"
#include "HDDMenuActions.h"
#include "EepromEditMenuActions.h"
#include "MenuActions.h"
#include "IdeDriver.h"

#include "lwip/apps/httpd.h"
#include "lwip/tcp.h"

bool netFlashOver;
static WebServerOps currentWebServerOp;
unsigned char* postProcessBuf;
unsigned int postProcessBufSize;
struct tcp_pcb *httpdPcb;

void NetworkManager_httpdInit(WebServerOps flashType)
{
    currentWebServerOp = flashType;
    postProcessBuf = NULL;
    postProcessBufSize = 0;
    netFlashOver = false;
    httpdPcb = NULL;

    httpd_init();
}

void NetworkManager_httpdHalt(void)
{
    if(netFlashOver)
    {
        tcp_arg(httpdPcb, NULL);
        tcp_sent(httpdPcb, NULL);
        tcp_recv(httpdPcb, NULL);
        tcp_err(httpdPcb, NULL);
        tcp_poll(httpdPcb, NULL, 0);
        if(ERR_OK != tcp_close(httpdPcb))
        {
            XBlastLogger(DEBUG_NETWORK, DBG_LVL_WARN, "Could not close httpd cleanly.");
            if(NULL != httpdPcb)
            {
                tcp_abort(httpdPcb);
            }
        }
        httpdPcb = NULL;
    }
}


bool newPostProcessData(WebServerOps op, const unsigned char* buf, unsigned int size)
{
    //op param will be used in the future
    //Maybe chain up operations?

    // Do not overwrite a pending operation
    if(postProcessBuf != NULL || postProcessBufSize != 0)
    {
        return false;
    }

    postProcessBuf = (unsigned char*)malloc(size);
    memcpy(postProcessBuf, buf, size);
    postProcessBufSize = size;

    return true;
}

bool netflashPostProcess(void)
{
    extern void ClearScreen (void);

    if(netFlashOver)
    {
        switch(currentWebServerOp)
        {
        case WebServerOps_BIOSFlash:
            ClearScreen ();
            FlashFileFromBuffer(postProcessBuf, postProcessBufSize, false);
            break;
        case WebServerOps_EEPROMFlash:
            ClearScreen ();
            updateEEPROMEditBufferFromInputBuffer(postProcessBuf, postProcessBufSize, true);
            UIFooter();
            break;
        case WebServerOps_HDD0Lock:
            ClearScreen ();

            if(IdeDriver_LockSecurityLevel_Disabled != IdeDriver_GetSecurityLevel(0))     //Drive is already locked
            {
                UnlockHDD(0, 0, postProcessBuf, false);    //Attempt Unlock only if SECURITY_UNLOCK was successful.
            }
            else
            {
                LockHDD(0, 0, postProcessBuf);
            }
            break;
        case WebServerOps_HDD1Lock:
            ClearScreen ();

            if(IdeDriver_LockSecurityLevel_Disabled != IdeDriver_GetSecurityLevel(1))     //Drive is already locked
            {
                UnlockHDD(1, 1, postProcessBuf, false);       //Attempt Unlock only if SECURITY_UNLOCK was successful.
            }
            else
            {
                LockHDD(1, 1, postProcessBuf);
            }
            break;
        }

        free(postProcessBuf);
        postProcessBuf = NULL;
        postProcessBufSize = 0;

        return true;
    }

    return false;
}

WebServerOps getWebServerOps(void)
{
    return currentWebServerOp;
}
