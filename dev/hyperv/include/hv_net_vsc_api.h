/*****************************************************************************
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The following copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Copyright (c) 2010-2011, Citrix, Inc.
 *
 * HyperV netvsc API header
 *
 *****************************************************************************/

#ifndef __HV_NET_VSC_API_H__
#define __HV_NET_VSC_API_H__

#ifdef REMOVED
/* Fixme -- removed */
#include "vmbusvar.h"
#endif

typedef void (*PFN_ON_SENDRECVCOMPLETION) (void *);

#define NETVSC_DEVICE_RING_BUFFER_SIZE 64*PAGE_SIZE //netvsc_drv_freebsd  -> 122
#define NETVSC_PACKET_MAXPAGE 4

typedef struct  {
	DLIST_ENTRY             ListEntry;
	DEVICE_OBJECT *Device;
	BOOL	IsDataPacket;  // One byte
	XFERPAGE_PACKET		*XferPagePacket;
	
	union {
		struct {
			UINT64  ReceiveCompletionTid;
			PVOID	ReceiveCompletionContext;
			PFN_ON_SENDRECVCOMPLETION OnReceiveCompletion;
		} Recv;	
		struct {
			UINT64 	 SendCompletionTid; //RndisFilter.c: 383
			PVOID	SendCompletionContext; //RndisFilter.c  : 381 -> 72 &  				RNDIS_REQUEST*  
			PFN_ON_SENDRECVCOMPLETION   OnSendCompletion;
		} Send;	
	} Completion;

	PVOID  Extension;  //netvsc_drv.c : 1240
	UINT32	TotalDataBufferLength; //long int  1242   ->1193
	UINT32	PageBufferCount; //netvsc_drv.c : 1241 -> 1192
	PAGE_BUFFER PageBuffers[NETVSC_PACKET_MAXPAGE];
} NETVSC_PACKET;


typedef struct {
	DRIVER_OBJECT	Base;
	UINT32		RingBufferSize; //netvsc_drv_freebsd.c  : 197->122
	UINT32		RequestExtSize;  //RndisFilter.c  : 768 -> 1120  size assumption
	UINT32		AdditionalRequestPageBufferCount; //netvsc_drv_freebsd.c 		: 515
	UINT32		(*OnReceiveCallback) (DEVICE_OBJECT *, NETVSC_PACKET *);
	VOID		(*OnLinkStatusChanged)(DEVICE_OBJECT *, UINT32) ;
	UINT32		(*OnOpen)(DEVICE_OBJECT *);
	UINT32		(*OnClose)(DEVICE_OBJECT *);
	UINT32		(*OnSend)(DEVICE_OBJECT *,NETVSC_PACKET* );
	PVOID		context;
} NETVSC_DRIVER_OBJECT;	

typedef struct {
	UCHAR	MacAddr[6];  //Assumption unsigned long 
	BOOL	LinkState;  //RndisFilter.c:  1032->62
} NETVSC_DEVICE_INFO;	


/*
 * ported from sys/nic/ns_hn.h (NetScaler-only file)
 */

typedef struct hn_softc {
	struct ifnet    *hn_ifp;
	struct arpcom   arpcom;
	device_t        hn_dev;
	u_int8_t        hn_unit;
	int             hn_carrier;
	int             hn_if_flags;
	struct mtx      hn_lock;
	vm_offset_t     hn_vaddr;
	int             hn_initdone;
	int             hn_xc;
	DEVICE_OBJECT   *hn_dev_obj;
	int             hn_cb_status;
	uint64_t        hn_sts_err_tx_nobufs;
	uint64_t        hn_sts_err_tx_enxio; //device not ready to xmit
	uint64_t        hn_sts_err_tx_eio;   //device not ready to xmit
} hn_softc_t;


/*
 * Externs
 */
extern int promisc_mode;

extern int  NetVscSetMode(DEVICE_OBJECT *Device, int mode);
extern void NetVscOnChannelCallback2(PVOID Context, int rxlimit);
/* NetScaler extension:  Do we need this? */
extern int  NetVscRxReady(PVOID Context);
extern void NetVscOnReceiveCompletion(PVOID Context);


#endif  /* __HV_NET_VSC_API_H__ */

