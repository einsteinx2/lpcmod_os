/*
 * WebBoot
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
#include <shared.h>

static char *requestGET;
static char *requestHEAD;
static int i = 0, initrdSize = 0, kernelSize = 0, hLen = 0, contLen = 0, fraction = 0;
static int head = 1, progCheck = 0, eoh = 0, fileLen = 0;
static char *tempBuf = (u8*)INITRD_START;
static char c[4], *appendLine = NULL, *header = NULL;
static struct ip_addr ipaddr;
static struct tcp_pcb *pcb = NULL;
static struct pbuf *q;
static u16_t port = 80;
static u32_t colour = 0xffff0000;

static err_t handleKernel(void *arg, struct tcp_pcb *pcb, err_t err);
static err_t handleInitrd(void *arg, struct tcp_pcb *pcb, err_t err);
static err_t handleAppend(void *arg, struct tcp_pcb *pcb, err_t err);
static err_t recvKernel(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);
static err_t recvInitrd(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);
static err_t recvAppend(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);
static void connErr(void *arg, err_t err);
static void boot(void);
static void processHeader(void);
extern char *finalURL;
extern char *finalKernelPath;
extern char *finalInitrdPath;
extern char *finalAppendPath;

static void processHeader() {
	char *headerPtr = strstr(header, "Content-Length");
	if(headerPtr != NULL) {
		headerPtr+=16;
		contLen = simple_strtol(headerPtr, NULL, 10);
//		printk("Content-Length: %i", contLen);
		free(header);
		fraction = (contLen / 64);
	} else {
		// Couldn't find the Content-Length.
		fraction = 250000;
	}
}

static void connErr(void *arg, err_t err) {
	printk("\n           Connection error...");
	cromwellError();
	printk("\n");
	while(1);
}

// Boot the system.
static void boot() {
	free(requestGET);
	free(requestHEAD);
	eth_disable();
	ExittoLinuxFromNet(initrdSize, appendLine);
}

static err_t recvKernel(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
	tcp_recved(pcb, p->tot_len);
	if(p == NULL) {
		tcp_close(pcb);
		if(head == 0) {
			//printk("Kernel downloaded (%i).\n", fileLen);
			memPlaceKernel(tempBuf, fileLen);
			//printk("Kernel Loaded.\n");
			cromwellSuccess();
			printk("\n");
			
			colour = 0xff00ff00;
			free(requestGET);
			free(requestHEAD);
			requestGET = (char *)malloc(1024);
			requestHEAD = (char *)malloc(1024);
			memset(requestGET, 0, 1024);
			memset(requestHEAD, 0, 1024);
			sprintf(requestGET, "GET %s%s HTTP/1.0\n\n", finalURL, finalInitrdPath);
			sprintf(requestHEAD, "HEAD %s%s HTTP/1.0\n\n", finalURL, finalInitrdPath);
			contLen = progCheck = hLen = fileLen = eoh = 0;
			head = 1;
			contLen = 11*1024*1024;
			fraction = contLen/64;

			if(header != NULL) {
				free(header);
				header = NULL;
			}
			header = (char*)malloc(5120);

			memset(tempBuf, 0, 15*1024*1024);

			pcb = tcp_new();
			tcp_err(pcb, connErr);
			tcp_setprio(pcb, TCP_PRIO_MAX);
			tcp_connect(pcb, &ipaddr, port, handleInitrd);
		} else {
			head = 0;
//			printk("Header:\n%s\n", header);
			processHeader();
			pcb = tcp_new();
			tcp_err(pcb, connErr);
			tcp_setprio(pcb, TCP_PRIO_MAX);
			tcp_connect(pcb, &ipaddr, port, handleKernel);
		}

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

			if(head == 1) {
				header[hLen] = c[3];
				hLen++;
				header[hLen] = '\0';
			}

			if(eoh == 1) {
				if(head == 0) {
					tempBuf[fileLen] = c[3];
					fileLen++;
					tempBuf[fileLen] = '\0';
					if((fileLen > progCheck) || (fileLen >= contLen)) {
						DisplayProgressBar(fileLen, contLen, colour);
						progCheck += fraction;
					}
				}
			}
			
			// Found the header.
			if(eoh == 0) {
				if((c[0] == '\r') && (c[1] == '\n') && (c[2] == '\r') && (c[3] == '\n')) {
					if(head == 0) {
						eoh = 1;
					}
					printk("...");
				}
			}
		}
	}
	pbuf_free(p);
	return ERR_OK;
}	

static err_t recvInitrd(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
	tcp_recved(pcb, p->tot_len);
	if(p == NULL) {
		tcp_close(pcb);
		if(head == 0) {
			//printk("Initrd downloaded (%i).\n", fileLen);
			initrdSize = fileLen;
			cromwellSuccess();
			printk("\n");

			colour = 0xff0000ff;
			free(requestGET);
			free(requestHEAD);
			requestGET = (char *)malloc(1024);
			requestHEAD = (char *)malloc(1024);
			memset(requestGET, 0, 1024);
			memset(requestHEAD, 0, 1024);
			sprintf(requestGET, "GET %s%s HTTP/1.0\n\n", finalURL, finalAppendPath);
			sprintf(requestHEAD, "HEAD %s%s HTTP/1.0\n\n", finalURL, finalAppendPath);
			contLen = progCheck = hLen = fileLen = eoh = 0;
			head = 1;
			contLen = 11*1024*1024;
			fraction = contLen/64;

			if(header != NULL) {
				free(header);
				header = NULL;
			}
			header = (char*)malloc(5120);

			pcb = tcp_new();
			tcp_err(pcb, connErr);
			tcp_setprio(pcb, TCP_PRIO_MAX);
			tcp_connect(pcb, &ipaddr, port, handleAppend);
		} else {
			head = 0;
//			printk("Header:\n%s\n", header);
			processHeader();
			pcb = tcp_new();
			tcp_err(pcb, connErr);
			tcp_setprio(pcb, TCP_PRIO_MAX);
			tcp_connect(pcb, &ipaddr, port, handleInitrd);
		}

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

			if(head == 1) {
				header[hLen] = c[3];
				hLen++;
				header[hLen] = '\0';
			}

			if(eoh == 1) {
				if(head == 0) {
					tempBuf[fileLen] = c[3];
					fileLen++;
					tempBuf[fileLen] = '\0';
					if((fileLen > progCheck) || (fileLen >= contLen)) {
						DisplayProgressBar(fileLen, contLen, colour);
						progCheck += fraction;
					}
				}
			}
			
			// Found the header.
			if(eoh == 0) {
				if((c[0] == '\r') && (c[1] == '\n') && (c[2] == '\r') && (c[3] == '\n')) {
					if(head == 0) {
						eoh = 1;
					}
					printk("...");
				}
			}
		}
	}
	pbuf_free(p);
	return ERR_OK;
}	

static err_t recvAppend(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
	tcp_recved(pcb, p->tot_len);
	if(p == NULL) {
		tcp_close(pcb);
		if(head == 0) {
			//printk("Append downloaded (%i).\n", fileLen);
			//printk("append: %s", appendLine);
			cromwellSuccess();
			printk("\n");
			boot();
		} else {
			head = 0;
//			printk("Header:\n%s\n", header);
			processHeader();
			appendLine = (char*)malloc(contLen+1);
			pcb = tcp_new();
			tcp_err(pcb, connErr);
			tcp_setprio(pcb, TCP_PRIO_MAX);
			tcp_connect(pcb, &ipaddr, port, handleAppend);
		}

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

			if(head == 1) {
				header[hLen] = c[3];
				hLen++;
				header[hLen] = '\0';
			}

			if(eoh == 1) {
				if(head == 0) {
					appendLine[fileLen] = c[3];
					fileLen++;
					appendLine[fileLen] = '\0';
					if((fileLen > progCheck) || (fileLen >= contLen)) {
						DisplayProgressBar(fileLen, contLen, colour);
						progCheck += fraction;
					}
				}
			}
			
			// Found the header.
			if(eoh == 0) {
				if((c[0] == '\r') && (c[1] == '\n') && (c[2] == '\r') && (c[3] == '\n')) {
					if(head == 0) {
						eoh = 1;
					}
					printk("...");
				}
			}
		}
	}
	pbuf_free(p);
	return ERR_OK;
}	

static err_t handleKernel(void *arg, struct tcp_pcb *pcb, err_t err) {
	tcp_recv(pcb, recvKernel);
	if(head == 1) {
		printk("           URL: %s%s\n", finalURL, finalKernelPath);
		printk("           Contacting server");
		tcp_write(pcb, requestHEAD, strlen(requestHEAD), 0);
	} else {
		cromwellSuccess();
		printk("           Downloading Kernel");
		tcp_write(pcb, requestGET, strlen(requestGET), 0);
	}		
	return ERR_OK;
}

static err_t handleInitrd(void *arg, struct tcp_pcb *pcb, err_t err) {
	tcp_recv(pcb, recvInitrd);
	if(head == 1) {
		printk("           URL: %s%s\n", finalURL, finalInitrdPath);
		printk("           Contacting server");
		tcp_write(pcb, requestHEAD, strlen(requestHEAD), 0);
	} else {
		cromwellSuccess();
		printk("           Downloading Initrd");
		tcp_write(pcb, requestGET, strlen(requestGET), 0);
	}		
	return ERR_OK;
}

static err_t handleAppend(void *arg, struct tcp_pcb *pcb, err_t err) {
	tcp_recv(pcb, recvAppend);
	if(head == 1) {
		printk("           URL: %s%s\n", finalURL, finalAppendPath);
		printk("           Contacting server");
		tcp_write(pcb, requestHEAD, strlen(requestHEAD), 0);
	} else {
		cromwellSuccess();
		printk("           Downloading Append");
		tcp_write(pcb, requestGET, strlen(requestGET), 0);
	}		
	return ERR_OK;
}

void webboot_init(int A, int B, int C, int D, int P) {
	requestGET = (char *)malloc(1024);
	requestHEAD = (char *)malloc(1024);
	memset(requestGET, 0, 1024);
	memset(requestHEAD, 0, 1024);
	sprintf(requestGET, "GET %s%s HTTP/1.0\n\n", finalURL, finalKernelPath);
	sprintf(requestHEAD, "HEAD %s%s HTTP/1.0\n\n", finalURL, finalKernelPath);
	contLen = progCheck = hLen = fileLen = eoh = 0;
	head = 1;
	contLen = 11*1024*1024;
	fraction = contLen/64;

	if(header != NULL) {
		free(header);
		header = NULL;
	}
	header = (char*)malloc(5120);

	memset(tempBuf, 0, 15*1024*1024);

	// Set the IP.
	IP4_ADDR(&ipaddr, A,B,C,D);
	port = (u16_t)P;

	if(pcb != NULL) {
		tcp_abort(pcb);
	}

	pcb = tcp_new();
	tcp_err(pcb, connErr);
	tcp_setprio(pcb, TCP_PRIO_MAX);
	tcp_connect(pcb, &ipaddr, port, handleKernel);
	cromwellSuccess();
	printk("           Server: %i.%i.%i.%i:%i\n\n", A, B, C, D, P);
	downloadingLED();
}
