/*
 * FtpServer.c
 *
 *  Created on: May 18, 2018
 *      Author: cromwelldev
 */

#include "FtpServer.h"
#include "ftpd.h"

void NetworkManager_ftpdInit(void)
{
    ftpd_init();
}

void NetworkManager_ftpdHalt(void)
{

}
