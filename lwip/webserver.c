/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#include "boot.h"
#include "lwip/debug.h"
#include "include/lpcmod_v1.h"
#include "lwip/stats.h"
#include "string.h"
#include "stdlib.h"
#include "lib/cromwell/cromString.h"
#include "HDDMenuActions.h"
#include "EepromEditMenuActions.h"
#include "Gentoox.h"
#include "MenuActions.h"
#include "BootFlash.h"
#include "httpd.h"

#include "lwip/tcp.h"

struct http_state {
	unsigned char retries;
	char *file;
	int left;
	
	int ispost;
	char *postdata;
	int postlen;
	int postpos;

	char lineBuf[256];
	char lineBufpos;
	char gotfirst;

	void *bios_start;
	int bios_len;

};

static char http_file0[] = 
#include "webContent/main.html.h"
;
static char http_file1[] = 
#include "webContent/ok.html.h"
;
static char http_file2[] = 
#include "webContent/fail.html.h"
;

static char http_file5[] =
#include "webContent/mainEEPROM.html.h"
;
static char http_file6[] =
#include "webContent/okEEPROM.html.h"
;
static char http_file7[] =
#include "webContent/failEEPROM.html.h"
;
static char http_file8[] =
#include "webContent/mainHDDLOCK.html.h"
;
static char http_file9[] =
#include "webContent/okHDDLOCK.html.h"
;
static char http_file10[] =
#include "webContent/failHDDLOCK.html.h"
;
static char http_file404[] = "HTTP/1.1 404 NOT FOUND\nContent-Type: text/html\nContent-Length: 11\n\nHello 404!\n";
static char http_file500[] = "HTTP/1.1 500 Internal Server Error\nContent-Type: text/html\nContent-Length: 11\n\nError 500!\n";

struct http_file {
	int len;
	char *data;
};
static struct http_file http_files[11]={
    {sizeof (http_file0) - 1, http_file0},       /* 0 */
    {sizeof (http_file1) - 1, http_file1},       /* 1 */
    {sizeof (http_file2) - 1, http_file2},       /* 2 */
    {sizeof (http_file404) - 1, http_file404},   /* 3 */
    {sizeof (http_file500) - 1, http_file500},   /* 4 */
    {sizeof (http_file5) - 1, http_file5},       /* 5 */
    {sizeof (http_file6) - 1, http_file6},       /* 6 */
    {sizeof (http_file7) - 1, http_file7},       /* 7 */
    {sizeof (http_file8) - 1, http_file8},       /* 8 */
    {sizeof (http_file9) - 1, http_file9},       /* 9 */
    {sizeof (http_file10) - 1, http_file10}       /* 10 */
};

/*-----------------------------------------------------------------------------------*/
static void
conn_err(void *arg, err_t err)
{
  struct http_state *hs;

  hs = arg;
  mem_free(hs);
}
/*-----------------------------------------------------------------------------------*/
static void close_conn(struct tcp_pcb *pcb, struct http_state *hs) {
  tcp_arg(pcb, NULL);
  tcp_sent(pcb, NULL);
  tcp_recv(pcb, NULL);
  unsigned char * fileBuf;

  if (hs->bios_start) {
        extern void ClearScreen (void);
        ClearScreen ();
        busyLED();
        //printk ("\nGot BIOS-image over http, %d bytes\n", hs->bios_len);
        switch(pcb->flashType){
            case WebServerOps_BIOSFlash:
                fileBuf = (unsigned char *) malloc (1024 * 1024);  //1MB buffer(max BIOS size)
                memset(fileBuf, 0x00, 1024 * 1024);   //Fill with 0.
                memcpy(fileBuf, hs->bios_start, hs->bios_len);
                netFlashOver = FlashFileFromBuffer(fileBuf, hs->bios_len, 0); //0 because we don't want to show confirmDialog screens.
                free(fileBuf);
                break;
            case WebServerOps_EEPROMFlash:
                updateEEPROMEditBufferFromInputBuffer(hs->bios_start, hs->bios_len, true);
                netFlashOver = 1;
                UIFooter();
                break;
            case WebServerOps_HDD0Lock:
                if((tsaHarddiskInfo[0].m_securitySettings &0x0002)==0x0002) {       //Drive is already locked
                    UnlockHDD(0, 1, (unsigned char *)hs->bios_start, false);                      //Attempt Unlock only if SECURITY_UNLOCK was successful.
                }
                else {
                    LockHDD(0, 1, (unsigned char *)hs->bios_start);
                }
                netFlashOver = 1;
                break;
            case WebServerOps_HDD1Lock:
                if((tsaHarddiskInfo[1].m_securitySettings &0x0002)==0x0002) {       //Drive is already locked
                    UnlockHDD(1, 1, (unsigned char *)hs->bios_start,false);                      //Attempt Unlock only if SECURITY_UNLOCK was successful.
                }
		else {
		    LockHDD(1, 1, (unsigned char *)hs->bios_start);
		}
                netFlashOver = 1;
                break;
            default:
                while(1);       //Just hang there.
                break;
        }

  }

  if (hs->postdata)
      free (hs->postdata);

  mem_free(hs);
  tcp_close(pcb);
}
/*-----------------------------------------------------------------------------------*/
static void
send_data(struct tcp_pcb *pcb, struct http_state *hs)
{
  err_t err;
  u16_t len;

  /* We cannot send more data than space available in the send
     buffer. */     
  if (tcp_sndbuf(pcb) < hs->left) {
    len = tcp_sndbuf(pcb);
  } else {
    len = hs->left;
  }

  do {
    err = tcp_write(pcb, hs->file, len, 0);
    if (err == ERR_MEM) {
      len /= 2;
    }
  } while (err == ERR_MEM && len > 1);  
  
  if (err == ERR_OK) {
    hs->file += len;
    hs->left -= len;
    /*  } else {
	printf("send_data: error %s len %d %d\n", lwip_strerr(err), len, tcp_sndbuf(pcb));*/
  }
}
/*-----------------------------------------------------------------------------------*/
static err_t
http_poll(void *arg, struct tcp_pcb *pcb)
{
  struct http_state *hs;

  hs = arg;
  
  /*  printf("Polll\n");*/
  if (hs == NULL) {
	  printk("http_poll");
	  /*tcp_abort(pcb);*/
    return ERR_OK;
  } else {
	  printk("HTTP_POLL");
    ++hs->retries;
    if (hs->retries == 4) {
      tcp_abort(pcb);
      return ERR_ABRT;
    }
    send_data(pcb, hs);
  }

  return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
static err_t
http_sent(void *arg, struct tcp_pcb *pcb, u16_t len)
{
  struct http_state *hs;

  hs = arg;

  hs->retries = 0;
  
  if (hs->left > 0) {    
    send_data(pcb, hs);
  } else {
    close_conn(pcb, hs);
  }

  return ERR_OK;
}


static int
handle_line(struct tcp_pcb *pcb, struct http_state *hs)
{
    unsigned char fileSelect;
	if (!hs->gotfirst) {
		if (strncmp (hs->lineBuf, "GET /", 4) == 0) {
			unsigned long fno = strtoul (&hs->lineBuf[5], NULL, 0);
			if (fno > 2) {
				fno = 3; /* 404 */
			}
			if(pcb->flashType == WebServerOps_HDD0Lock || pcb->flashType == WebServerOps_HDD1Lock){
			    fileSelect = 8;
			}
			else{
			    fileSelect = pcb->flashType * 5;
			}
			hs->file = http_files[fno + (fileSelect)].data;
			hs->left = http_files[fno + (fileSelect)].len;
		} else if (strncmp (hs->lineBuf, "POST /", 5) == 0) {
			hs->ispost = 1;
			hs->file = http_files[4].data;
			hs->left = http_files[4].len;
		} else {
			return 0;
		}
		hs->gotfirst = 1;
	} else {

		/* end of header empty line? */
		if (hs->lineBuf[0] == '\0') {
			if (hs->ispost && hs->postlen) {
				hs->postdata = (char *)malloc (hs->postlen);
				hs->postpos = 0;
			} else {
				send_data(pcb, hs);
				tcp_poll(pcb, http_poll, 4);
				//printk("\n  handle_line");
				tcp_sent(pcb, http_sent);
				hs->gotfirst = 0;
			}
		}

		if (strncmp (hs->lineBuf, "Content-Length:", 15) == 0) {
			unsigned long len = strtoul (&hs->lineBuf[16], NULL, 0);

			hs->postlen = len;
		}
	}
	return 1;
}

char * xstrstr(const char * s1, const char * s2)
{
	int l1, l2;

	l2 = strlen(s2);
	if (!l2)
		return (char *) s1;
	l1 = strlen(s1);

	return NULL;
}

static int
handle_post(struct http_state *hs, unsigned char flashType)
{
	int i, ncnt = 0, blen, len;
	char *start, *end;
	char *boundary = NULL;

	hs->file = http_files[4].data;
	hs->left = http_files[4].len;

	for (i = 0; i < hs->postlen; i++) {
		if (hs->postdata[i] == '\n' || hs->postdata[i] == '\r') {
			hs->postdata[i] = 0;
			boundary = hs->postdata;
			blen = i;
			break;
		}
	}
	if (!boundary)
		return 0;

	for (; i < hs->postlen; i++) {
		if (hs->postdata[i] == '\r')
			continue;

		if (hs->postdata[i] == '\n') {
			ncnt++;
			if (ncnt == 2) {
				start = &hs->postdata[i+1];
				break;
			}
		} else {
			ncnt = 0;
		}
	}
	if (!start) {
		printk ("Could not find start...\n");
		return 0;
	}

	for (; i < hs->postlen; i++) {
		if ( memcmp(boundary,&hs->postdata[i],blen) == 0) {
			end = &hs->postdata[i];
			break;
		}
	}

	if (!end) {
		printk ("Could not find end...\n");
		return 0;
	}

	end -= 2;

	len = end - start;

	switch(flashType){
	case WebServerOps_BIOSFlash:
	    if(len != 256*1024 && len != 512*1024 && len != 1024*1024){
	        hs->file = http_files[2].data;
		hs->left = http_files[2].len;
		printk ("Illegal size, NOT flashing\n");
		return 0;
	    }
	    hs->file = http_files[1].data;
	    hs->left = http_files[1].len;
	    break;
	case WebServerOps_EEPROMFlash:
	    if(len != 256){
	        hs->file = http_files[7].data;
		hs->left = http_files[7].len;
		printk ("Illegal size, NOT flashing\n");
		return 0;
	    }
	    hs->file = http_files[6].data;
	    hs->left = http_files[6].len;
	    break;
	case WebServerOps_HDD0Lock:
	case WebServerOps_HDD1Lock:
            if(len != 256){
	        hs->file = http_files[10].data;
		hs->left = http_files[10].len;
		printk ("Illegal size, NOT flashing\n");
		return 0;
	    }
	    hs->file = http_files[9].data;
	    hs->left = http_files[9].len;
	    break;
	default:
		printk ("Unkown operation. Halting.\n");
		return 0;
	    break;
	}

	hs->bios_start = start;
	hs->bios_len = len;

	return 1;
}
/*-----------------------------------------------------------------------------------*/
static err_t
http_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
  int i;
  char *data;
  struct http_state *hs;

  hs = arg;

  if (err == ERR_OK && p != NULL)
  {
	  struct pbuf *q;

	  debugSPIPrint("Received pbuf.");
    /* Inform TCP that we have taken the data. */
    tcp_recved(pcb, p->tot_len);

	  for (q = p; q; q = q->next) {
		  for (i = 0; i < q->len; i++) {
			  char c = ((char *)q->payload)[i];

			  if (!hs->postdata) {
				  if (c == '\r') /* ignore \r */
					  continue;
				  
				  if (c == '\n') {
					  hs->lineBuf[hs->lineBufpos] = 0;
					  if (!handle_line (pcb, hs)) {
						  pbuf_free(p);
						  close_conn(pcb, hs);
						  return ERR_OK;
					  }
					  hs->lineBufpos = 0;
				  } else {
					  if (hs->lineBufpos < sizeof (hs->lineBuf) - 2)
						  hs->lineBuf[hs->lineBufpos++] = c;
				  }
			  } else {
				  hs->postdata[hs->postpos++] = c;
				  if (hs->postpos == hs->postlen) {
					  handle_post(hs, pcb->flashType);
					  send_data(pcb, hs);
					  tcp_poll(pcb, http_poll, 4);
					  printk("\n  http_recv");
					  tcp_sent(pcb, http_sent);
					  hs->gotfirst = 0;
				  }
			  }
		  }
	  }

	pbuf_free(p);
  }
  if (err == ERR_OK && p == NULL)
  {
	  debugSPIPrint("pbuf null. Closing connection.");
    close_conn(pcb, hs);
  }
  return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
static err_t
http_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{
  struct http_state *hs;

  tcp_setprio(pcb, TCP_PRIO_MIN);
  
  /* Allocate memory for the structure that holds the state of the
     connection. */
  hs = mem_malloc(sizeof(struct http_state));

  if (hs == NULL) {
    printk("http_accept: Out of memory\n");
    return ERR_MEM;
  }

  /* Initialize the structure. */
  hs->file = NULL;
  hs->left = 0;
  hs->retries = 0;
  hs->lineBufpos = 0;
  hs->gotfirst = 0;
  hs->ispost = 0;
  hs->postdata = NULL;
  hs->postpos = 0;
  hs->postlen = 0;

  hs->bios_start = NULL;
  hs->bios_len = 0;

  /* Tell TCP that this is the structure we wish to be passed for our
     callbacks. */
  tcp_arg(pcb, hs);

  /* Tell TCP that we wish to be informed of incoming data by a call
     to the http_recv() function. */
  tcp_recv(pcb, http_recv);

  tcp_err(pcb, conn_err);
  
  return ERR_OK;
}
/*-----------------------------------------------------------------------------------*/
void
httpd_init(unsigned char flashType)
{
  struct tcp_pcb *pcb;

  pcb = tcp_new(flashType);
  tcp_bind(pcb, IP_ADDR_ANY, 80);
  pcb = tcp_listen(pcb, flashType);
  tcp_accept(pcb, http_accept);
  //printk("httpd_init");
  //cromwellSuccess();
  //downloadingLED();
}
/*-----------------------------------------------------------------------------------*/
