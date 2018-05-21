/*
 * FtpServer.c
 *
 *  Created on: May 18, 2018
 *      Author: cromwelldev
 */

#include "FtpServer.h"
#include "ftpd.h"
#include "lib/LPCMod/xblastDebug.h"
#include "lwip/tcp.h"
#include "stddef.h"

struct tcp_pcb *ftpdPcb;

void NetworkManager_ftpdInit(void)
{
    ftpd_init();
}

#if 0
unsigned char NetworkManager_ftpdHalt(void)
{
    if(1)
    {
        tcp_arg(ftpdPcb, NULL);
        tcp_sent(ftpdPcb, NULL);
        tcp_recv(ftpdPcb, NULL);
        tcp_err(ftpdPcb, NULL);
        tcp_poll(ftpdPcb, NULL, 0);
        if(ERR_OK != tcp_close(ftpdPcb))
        {
            XBlastLogger(DEBUG_NETWORK, DBG_LVL_WARN, "Could not close ftpd cleanly.");
            if(NULL != ftpdPcb)
            {
                tcp_abort(ftpdPcb);
            }
        }
        ftpdPcb = NULL;
        return 1;
    }
    else
    {
        XBlastLogger(DEBUG_NETWORK, DBG_LVL_WARN, "Cannot close. ftp in use.");
    }

    return 0;
}
#endif
