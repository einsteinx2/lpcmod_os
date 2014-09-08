/*
 * WebUpdate
 * Copyright (C) Thomas "ShALLaX" Pedley (gentoox@shallax.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "boot.h"
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/tcp.h"
#include "memory_layout.h"

static const char *requestBIOS  = "GET /sourceforge/xbox-linux/loader.latest HTTP/1.0\n\n";
static int i = 0, contLen = 0, fraction = 0, progCheck = 0, eoh = 0, fileLen = 0;
static char *tempBuf = (u8*)INITRD_START;
static char c[4];
static struct ip_addr ipaddr;
static struct tcp_pcb *pcb = NULL;
static struct pbuf *q;
static u16_t port = 80;

static err_t handleBIOS(void *arg, struct tcp_pcb *pcb, err_t err);
static err_t recvBIOS(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);
static void connErr(void *arg, err_t err);
void flashBIOS(void);

static void connErr(void *arg, err_t err) {
	printk("\n           Connection error");
	dots();
	cromwellError();
	printk("\n");
	while(1);
}

// Flash the BIOS.
void flashBIOS() {
	eth_disable();
	busyLED();
	//printk ("\n           Got BIOS-image over http, %d bytes\n", fileLen);
	if (fileLen != 256*1024 && fileLen != 512*1024 && fileLen != 1024*1024) {
		printk("\n           Error downloading file");
		dots();
		cromwellError();
		while(1);
   }

	extern void ClearScreen (void);
	ClearScreen ();
	memcpy ((void*)0x100000, tempBuf, fileLen);
	BootReflashAndReset((void*)0x100000,0,fileLen);
	while(1);

}

static err_t recvBIOS(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
	tcp_recved(pcb, p->tot_len);
	if(p == NULL) {
		tcp_close(pcb);
		flashBIOS();
	}
	if(eoh == 0) {
		c[0] = '\0';
		c[1] = '\0';
		c[2] = '\0';
		c[3] = '\0';
	}
	
	for (q = p; q; q = q->next) {
		for (i = 0; i < q->len; i++) {
			if(eoh == 0) {
				c[0] = c[1];
				c[1] = c[2];
				c[2] = c[3];
			}

			c[3] = ((char *)q->payload)[i];

			if(eoh == 1) {
				tempBuf[fileLen] = c[3];
				fileLen++;
				tempBuf[fileLen] = '\0';
				if((fileLen > progCheck) || (fileLen >= contLen)) {
					DisplayProgressBar(fileLen,contLen,0xffff0000);
					progCheck += fraction;
				}
			}
			
			// Found the header.
			if(eoh == 0) {
				if((c[0] == '\r') && (c[1] == '\n') && (c[2] == '\r') && (c[3] == '\n')) {
					eoh = 1;
					dots();
				}
			}
		}
	}
	pbuf_free(p);
	return ERR_OK;
}	

static err_t handleBIOS(void *arg, struct tcp_pcb *pcb, err_t err) {
	tcp_recv(pcb, recvBIOS);
	printk("           Downloading BIOS");
	tcp_write(pcb, requestBIOS, strlen(requestBIOS), 0);
	return ERR_OK;
}

void webupdate_init(void) {
	contLen = progCheck = fileLen = eoh = 0;
	contLen = 256*1024;
	fraction = contLen/16;

	memset(tempBuf, 0, 1024*1024);

	IP4_ADDR(&ipaddr, WF_BLOCK_A, WF_BLOCK_B, WF_BLOCK_C, WF_BLOCK_D);
	port = WF_PORT;

	if(pcb != NULL) {
		tcp_abort(pcb);
	}

	pcb = tcp_new();
	tcp_err(pcb, connErr);
	tcp_setprio(pcb, TCP_PRIO_MAX);
	tcp_connect(pcb, &ipaddr, port, handleBIOS);
	cromwellSuccess();
	printk("           Server: %i.%i.%i.%i:%i\n", WF_BLOCK_A, WF_BLOCK_B, WF_BLOCK_C, WF_BLOCK_D, WF_PORT);
	downloadingLED();
}
