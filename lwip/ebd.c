#include "lwip/stats.h"
#include "lwip/mem.h"
#include "netif/etharp.h"
#include "lwip/tcp.h"
#include "lwip/dhcp.h"
#include "httpd.h"
#include "boot.h"
#include "xblast/settings/xblastSettingsDefs.h"
#include "lib/cromwell/cromString.h"
#include "string.h"
#include <stdarg.h>

struct eth_addr ethaddr = { 0, 0x0d, 0xff, 0xff, 0, 0 };

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

static NetworkState currentNetworkState = NetworkState_Idle;
static struct ip_addr ipaddr, netmask, gw;
static struct netif *netif = NULL;
static bool first = 1;
static int divisor = 0;

void
eth_transmit (const char *d, unsigned int t, unsigned int s, const void *p);
int
eth_poll_into (char *buf, int *len);

static struct pbuf *
ebd_poll (struct netif *netif) {
    struct pbuf *p, *q;
    char *bufptr;
    char buf[1500];
    int len;

    if (!eth_poll_into (buf, &len))
        return NULL;

    debugSPIPrint("New data from eth: %u bytes", len);
    p = pbuf_alloc (PBUF_LINK, len, PBUF_POOL);
    if (p != NULL) {
        bufptr = &buf[0];
        for (q = p; q != NULL; q = q->next) {
        	debugSPIPrint("Copying %u bytes into pbuf", q->len);
            memcpy (q->payload, bufptr, q->len);
            bufptr += q->len;
        }
    }
    else {
        printk ("Could not allocate pbufs\n");
    }

    debugSPIPrint("pbuf total size: %u", p->tot_len);
    return p;
}

static err_t
ebd_low_level_output (struct netif *netif, struct pbuf *p) {
    char buf[1500];
    char *bufptr;
    struct eth_hdr *h;
    struct pbuf *q;

    bufptr = &buf[0];
    h = (struct eth_hdr *) bufptr;
    for (q = p; q != NULL; q = q->next) {
        memcpy (bufptr, q->payload, q->len);
        bufptr += q->len;
    }
    eth_transmit (&h->dest.addr[0], ntohs (h->type), p->tot_len - 14, &buf[14]);
    return 0;   //Keep compiler happy
}

static err_t
ebd_output (struct netif *netif, struct pbuf *p, struct ip_addr *ipaddr) {
    p = etharp_output (netif, ipaddr, p);
    if (p != NULL) {
        return ebd_low_level_output (netif, p);
    }
    else {
        printk ("b");
    }
    return ERR_OK;
}

int
ebd_wait (struct netif *netif, u16_t time) {
    unsigned long delay_ticks;
    static unsigned long start_ticks = 0;
    extern unsigned long
    currticks (void);

    delay_ticks = time * 3579;
    if (!start_ticks)
        start_ticks = currticks ();

    while (1) {
        struct eth_hdr *ethhdr;
        struct pbuf *p, *q;

        p = ebd_poll (netif);
        if (p) {
            ethhdr = p->payload;
            q = NULL;
            switch (htons (ethhdr->type)) {
                case ETHTYPE_IP:
                    q = etharp_ip_input (netif, p);
                    pbuf_header (p, -14);
                    netif->input (p, netif);
                    break;
                case ETHTYPE_ARP:
                    q = etharp_arp_input (netif, &ethaddr, p);
                    break;
                default:
                    pbuf_free (p);
                    break;
            }
            if (q != NULL) {
                ebd_low_level_output (netif, q);
                pbuf_free (q);
            }
            return 1;
        }
        else {
            unsigned long ticks = currticks () - start_ticks;
            if (ticks > delay_ticks) {
                start_ticks = 0;
                return 0;
            }
        }
    }
    return 0;   //Keep compiler happy
}

extern char forcedeth_hw_addr[6];

static err_t
ebd_init (struct netif *netif) {
    netif->hwaddr_len = 6;
    memcpy (netif->hwaddr, forcedeth_hw_addr, 6);
    memcpy (ethaddr.addr, forcedeth_hw_addr, 6);
    netif->name[0] = 'e';
    netif->name[1] = 'b';
    netif->output = ebd_output;
    netif->linkoutput = ebd_low_level_output;

    debugSPIPrint("netif populated. Interface name %c%c  MAC:%02X-%02X-%02X-%02X-%02X-%02X"  , netif->name[0], netif->name[1], netif->hwaddr[0], netif->hwaddr[1], netif->hwaddr[2], netif->hwaddr[3], netif->hwaddr[4], netif->hwaddr[5]);
    return ERR_OK;
}

int
run_lwip (unsigned char flashType) {

    bool returnResult = 0;

    switch(currentNetworkState)
    {
    case NetworkState_Idle:
		debugSPIPrint("currentNetworkState == NetworkState_Idle");
    	//Init a couple of Lwip globals because they seem to assume declared pointers are set to NULL by default.
		//From tcp.c
		tcp_listen_pcbs.listen_pcbs = NULL;
		tcp_active_pcbs = NULL;
		tcp_tw_pcbs = NULL;
		tcp_tmp_pcb = NULL;

		//from netif.c
		netif_list = NULL;
		netif_default = NULL;

		first = 1;
		divisor = 0;
		currentNetworkState = NetworkState_Init;
    	debugSPIPrint("currentNetworkState == NetworkState_Init");
    	break;
    case NetworkState_Init:
        mem_init ();
        memp_init ();
        pbuf_init ();
        netif_init ();
        ip_init ();
        udp_init ();
        tcp_init ();
        etharp_init ();

        printk ("\n            TCP/IP initialized.\n");
		netFlashOver = false;

		netif = netif_find("eb");   //Trying to find previously configured network interface

		if(netif == NULL)
		{
			debugSPIPrint("No configured network interface found. Creating one.");
			netif = (struct netif *)malloc(sizeof(struct netif));
			//Will never be removed for entire duration of program execution so no free()...

			//These will be overwritten by DHCP anyway if need be.
			IP4_ADDR(&gw, 0,0,0,0);
			IP4_ADDR(&ipaddr, 0,0,0,0);
			IP4_ADDR(&netmask, 255,255,255,255);

			netif_add (netif, &ipaddr, &netmask, &gw, NULL, ebd_init, ip_input);
		}
		else
		{
			debugSPIPrint("Found previously configured network interface.");
		}

		if (LPCmodSettings.OSsettings.useDHCP)
		{
			//Re-run DHCP discover anyways just in case lease was revoked.
			dhcp_start (netif);

			currentNetworkState = NetworkState_DHCPStart;
			debugSPIPrint("currentNetworkState == NetworkState_DHCPStart");
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

			currentNetworkState = NetworkState_ServerInit;
			debugSPIPrint("currentNetworkState == NetworkState_ServerInit");
		}
		netif_set_default (netif);
    	break;
    case NetworkState_DHCPStart:
    	if (!ebd_wait(netif, TCP_TMR_INTERVAL))
    	{
			if (divisor++ == 60 * 4)
			{
				if (first && netif->dhcp->state != DHCP_BOUND)
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

					currentNetworkState = NetworkState_ServerInit;
					debugSPIPrint("currentNetworkState == NetworkState_ServerInit");
				}
				first = 0;
				dhcp_coarse_tmr();
				divisor=0;
			}
			if (divisor & 1)
				dhcp_fine_tmr();
			tcp_tmr();
			//else
			//	printk("Got packet!! \n");
		}

    	if(netif->dhcp->state == DHCP_BOUND)
    	{
    		currentNetworkState = NetworkState_ServerInit;
			debugSPIPrint("currentNetworkState == NetworkState_ServerInit");
    	}
    	break;
    case NetworkState_ServerInit:
        printk ("\n\n            Go to 'http://%u.%u.%u.%u' to flash your BIOS.\n",
            ((netif->ip_addr.addr) & 0xff),
            ((netif->ip_addr.addr) >> 8 & 0xff),
            ((netif->ip_addr.addr) >> 16 & 0xff),
            ((netif->ip_addr.addr) >> 24 & 0xff));

		httpd_init(flashType);

		currentNetworkState = NetworkState_ServerRunning;
    	debugSPIPrint("currentNetworkState == NetworkState_ServerRunning");
    	break;
    case NetworkState_ServerRunning:
    	if (!ebd_wait(netif, TCP_TMR_INTERVAL))
		{
			if (divisor++ == 60 * 4)
			{
				dhcp_coarse_tmr();
				divisor=0;
			}
			if (divisor & 1)
				dhcp_fine_tmr();
			tcp_tmr();
			//else
			//	printk("Got packet!! \n");
		}

    	if(netFlashOver == true)
    	{
    		currentNetworkState = NetworkState_ServerShuttingDown;
			debugSPIPrint("currentNetworkState == NetworkState_ServerShuttingDown");
    	}
    	break;
    case NetworkState_ServerShuttingDown:
    	dhcp_stop(netif);

    	currentNetworkState = NetworkState_Cleanup;
    	debugSPIPrint("currentNetworkState == NetworkState_Cleanup");
    	break;
    case NetworkState_Cleanup:
    	returnResult = 1;

    	currentNetworkState = NetworkState_Idle;
    	break;
    }

    return returnResult;   //Keep compiler happy
}

