/*
 * NetworkManager.c
 *
 *  Created on: May 18, 2018
 *      Author: cromwelldev
 */

#include "NetworkManager.h"
#include "HttpServer.h"

#include "xblast/settings/xblastSettingsDefs.h"
#include "Gentoox.h"
#include "Gentoox.h"
#include "FlashDriver.h"
#include "video.h"
#include "string.h"
#include "stdio.h"
#include "lib/LPCMod/xblastDebug.h"
#include "lib/cromwell/CallbackTimer.h"

#include "lwip/init.h"
#include "lwip/priv/tcp_priv.h"
#include "lwip/dhcp.h"
#include "lwip/netif.h"
#include "lwip/timeouts.h"

static NetworkState currentNetworkState = NetworkState_Idle;
static bool nicInit = false;
static struct ip4_addr ipaddr, netmask, gw;
static struct netif *netif = NULL;
static unsigned int callbackTimerId;

#define DHCP_WAIT_MS 10000 /* Max allowed time as per RFC2131 */

static void dhcpFailCallback(void);
extern void ethernetif_input(struct netif *netif);
extern err_t ethernetif_init(struct netif *netif);
extern err_t ethernet_input(struct pbuf *p, struct netif *netif);
extern int etherboot(void);

void NetworkManager_init(void)
{
    //Init a couple of Lwip globals because they seem to assume declared pointers are set to NULL by default.
    //From tcp.c
    tcp_bound_pcbs = NULL;
    tcp_listen_pcbs.listen_pcbs = NULL;
    tcp_active_pcbs = NULL;
    tcp_tw_pcbs = NULL;
    //tcp_tmp_pcb = NULL; //Not needed anymore in 2.0.0

    //from netif.c
    netif_list = NULL;
    netif_default = NULL;

    //From udp.c
    udp_pcbs = NULL;

    XBlastLogger(DEBUG_LWIP, DBG_LVL_INFO, "init done.");
}

void NetworkManager_update(void)
{
    switch(currentNetworkState)
    {
    case NetworkState_Idle:

        break;
    case NetworkState_Init:
        lwip_init();

        netif = netif_find("eb");   //Trying to find previously configured network interface

        if(netif == NULL)
        {
            XBlastLogger(DEBUG_LWIP, DBG_LVL_DEBUG, "No configured network interface found. Creating one.");
            netif = (struct netif *)malloc(sizeof(struct netif));
            //Will never be removed for entire duration of program execution so no free()...

            //These will be overwritten by DHCP anyway if need be.
            IP4_ADDR(&gw, 0,0,0,0);
            IP4_ADDR(&ipaddr, 0,0,0,0);
            IP4_ADDR(&netmask, 0,0,0,0);

            netif_add (netif, &ipaddr, &netmask, &gw, NULL, ethernetif_init, ethernet_input);
        }
        else
        {
            XBlastLogger(DEBUG_LWIP, DBG_LVL_DEBUG, "Found previously configured network interface.");
        }

        netif_set_up(netif);

        if (LPCmodSettings.OSsettings.useDHCP)
        {
            //Re-run DHCP discover anyways just in case lease was revoked.
            dhcp_start (netif);

            callbackTimerId = newCallbackTimer(dhcpFailCallback, DHCP_WAIT_MS, IsSingleUseTimer);
            if(0 == callbackTimerId)
            {
                dhcpFailCallback();
            }
            else
            {
                currentNetworkState = NetworkState_DHCPStart;
                XBlastLogger(DEBUG_LWIP, DBG_LVL_DEBUG, "currentNetworkState == NetworkState_DHCPStart");
            }
        }
        else
        {
            //Not necessary, but polite.
            dhcp_stop (netif);
            IP4_ADDR(&gw, LPCmodSettings.OSsettings.staticGateway[0],
                     LPCmodSettings.OSsettings.staticGateway[1],
                     LPCmodSettings.OSsettings.staticGateway[2],
                     LPCmodSettings.OSsettings.staticGateway[3]);
            IP4_ADDR(&ipaddr, LPCmodSettings.OSsettings.staticIP[0],
                     LPCmodSettings.OSsettings.staticIP[1],
                     LPCmodSettings.OSsettings.staticIP[2],
                     LPCmodSettings.OSsettings.staticIP[3]);
            IP4_ADDR(&netmask, LPCmodSettings.OSsettings.staticMask[0],
                     LPCmodSettings.OSsettings.staticMask[1],
                     LPCmodSettings.OSsettings.staticMask[2],
                     LPCmodSettings.OSsettings.staticMask[3]);
            netif_set_addr(netif, &ipaddr, &netmask, &gw);

            dhcp_inform (netif);

            currentNetworkState = NetworkState_Running;
            XBlastLogger(DEBUG_LWIP, DBG_LVL_DEBUG, "currentNetworkState == NetworkState_Running");
        }
        netif_set_default (netif);
        break;
    case NetworkState_DHCPStart:
        ethernetif_input(netif);

        if(dhcp_supplied_address(netif))
        {
            stopCallbackTimer(callbackTimerId);
            currentNetworkState = NetworkState_Running;
            XBlastLogger(DEBUG_LWIP, DBG_LVL_DEBUG, "currentNetworkState == NetworkState_Running");
        }
        break;
    case NetworkState_Running:
        ethernetif_input(netif);

        break;
    case NetworkState_ShuttingDown:
        dhcp_stop(netif);
        currentNetworkState = NetworkState_Cleanup;
        XBlastLogger(DEBUG_LWIP, DBG_LVL_DEBUG, "currentNetworkState == NetworkState_Cleanup");
        break;
    case NetworkState_Cleanup:
        switch(getWebServerOps())
        {
        case WebServerOps_BIOSFlash:
        {
            FlashProgress flashProgress = Flash_getProgress();
            if(flashProgress.currentFlashOp == FlashOp_Idle)
            {
                currentNetworkState = NetworkState_Idle;
            }
        }
        break;
        default:
            currentNetworkState = NetworkState_Idle;
            break;
        }
        break;
    }

    sys_check_timeouts();
}

void NetworkManager_start(void)
{
    if(false == nicInit)
    {
        if(0 == etherboot())
        {
            nicInit = true;
        }
    }
    currentNetworkState = NetworkState_Init;
    XBlastLogger(DEBUG_LWIP, DBG_LVL_DEBUG, "currentNetworkState == NetworkState_Init");
}

void NetworkManager_stop(void)
{
    if(NetworkState_Running == currentNetworkState)
    {
        currentNetworkState = NetworkState_ShuttingDown;
    }
}

NetworkState NetworkManager_getState(void)
{
    return currentNetworkState;
}

unsigned char NetworkManager_getIP(char * sz_out)
{
    if(netif->flags & NETIF_FLAG_LINK_UP)
    {
        return sprintf(sz_out, "%u.%u.%u.%u",
                    ((netif->ip_addr.addr) & 0xff),
                    ((netif->ip_addr.addr) >> 8 & 0xff),
                    ((netif->ip_addr.addr) >> 16 & 0xff),
                    ((netif->ip_addr.addr) >> 24 & 0xff));
    }
    XBlastLogger(DEBUG_LWIP, DBG_LVL_DEBUG, "netif flag not UP");

    return 0;
}

static void dhcpFailCallback(void)
{
    if (dhcp_supplied_address(netif) == false)
    {
        //Not necessary, but polite.
        dhcp_stop (netif);
        IP4_ADDR(&gw, LPCmodSettings.OSsettings.staticGateway[0],
                 LPCmodSettings.OSsettings.staticGateway[1],
                 LPCmodSettings.OSsettings.staticGateway[2],
                 LPCmodSettings.OSsettings.staticGateway[3]);
        IP4_ADDR(&ipaddr, LPCmodSettings.OSsettings.staticIP[0],
                 LPCmodSettings.OSsettings.staticIP[1],
                 LPCmodSettings.OSsettings.staticIP[2],
                 LPCmodSettings.OSsettings.staticIP[3]);
        IP4_ADDR(&netmask, LPCmodSettings.OSsettings.staticMask[0],
                 LPCmodSettings.OSsettings.staticMask[1],
                 LPCmodSettings.OSsettings.staticMask[2],
                 LPCmodSettings.OSsettings.staticMask[3]);
        netif_set_addr(netif, &ipaddr, &netmask, &gw);

        dhcp_inform (netif);
    }
    currentNetworkState = NetworkState_Running;
    XBlastLogger(DEBUG_LWIP, DBG_LVL_DEBUG, "currentNetworkState == NetworkState_Running");
}
