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
 * HyperV vmbus private definition file
 *
 *****************************************************************************/

/*++

Copyright 2008 Microsoft Corporation. All Rights Reserved.

--*/

/*++

File:
	VmbusPrivate.h

Description:
	Vmbus private definition file 

--*/

#ifndef __HV_VMBUS_PRIVATE_H__
#define __HV_VMBUS_PRIVATE_H__

#ifndef INTERNAL
#define INTERNAL static
#endif

#ifdef REMOVED
/* Fixme:  Removed */
#include "Hv.h"
#include "VmbusApi.h"
#include "Channel.h"
#include "ChannelMgmt.h"
#include "ChannelInterface.h"
#include "ChannelMessages.h"
#include "RingBuffer.h"
//#include "Packet.h"
#include "List.h"
#include "timesync_ic.h"
#endif


//
// Defines
//

// Maximum channels is determined by the size of the interrupt page which is PAGE_SIZE. 1/2 of PAGE_SIZE is for
// send endpoint interrupt and the other is receive endpoint interrupt
#define MAX_NUM_CHANNELS				(PAGE_SIZE >> 1) << 3  // 16348 channels

// The value here must be in multiple of 32
// TODO: Need to make this configurable
#define MAX_NUM_CHANNELS_SUPPORTED		256

//
// Data types
//

typedef enum {
	Disconnected,
	Connecting,
	Connected,
	Disconnecting
} VMBUS_CONNECT_STATE;

#define MAX_SIZE_CHANNEL_MESSAGE			HV_MESSAGE_PAYLOAD_BYTE_COUNT

typedef struct _VMBUS_CONNECTION {

	VMBUS_CONNECT_STATE					ConnectState;

	UINT32								NextGpadlHandle;

	// Represents channel interrupts. Each bit position
	// represents a channel.
	// When a channel sends an interrupt via VMBUS, it 
	// finds its bit in the sendInterruptPage, set it and 
	// calls Hv to generate a port event. The other end
	// receives the port event and parse the recvInterruptPage
	// to see which bit is set
	VOID*								InterruptPage;
	VOID*								SendInterruptPage;
	VOID*								RecvInterruptPage;

	// 2 pages - 1st page for parent->child notification and 2nd is child->parent notification
	VOID*								MonitorPages;
	LIST_ENTRY							ChannelMsgList;
	HANDLE								ChannelMsgLock;

	// List of channels
	LIST_ENTRY							ChannelList;
	HANDLE								ChannelLock;

	HANDLE								WorkQueue;
} VMBUS_CONNECTION;


typedef struct _VMBUS_MSGINFO {
	// Bookkeeping stuff
	LIST_ENTRY			MsgListEntry;

	// Synchronize the request/response if needed
	HANDLE				WaitEvent;

	// The message itself
	unsigned char		Msg[0];
} VMBUS_MSGINFO;


//
// Externs
//
extern VMBUS_CONNECTION gVmbusConnection;

//
// General vmbus interface
//
INTERNAL DEVICE_OBJECT*
VmbusChildDeviceCreate(
	GUID deviceType,
	GUID deviceInstance,
	void *context);

INTERNAL int
VmbusChildDeviceAdd(
	DEVICE_OBJECT* Device);

INTERNAL void
VmbusChildDeviceRemove(
   DEVICE_OBJECT* Device);

//INTERNAL void
//VmbusChildDeviceDestroy(
//	DEVICE_OBJECT*);

INTERNAL VMBUS_CHANNEL*
GetChannelFromRelId(
	UINT32 relId
	);

//
// Connection interface
//
INTERNAL int
VmbusConnect(
	VOID
	);

INTERNAL int
VmbusDisconnect(
	VOID
	);

INTERNAL int
VmbusPostMessage(
	PVOID			buffer,
	SIZE_T			bufSize
	);

INTERNAL int
VmbusSetEvent(
	UINT32 childRelId
	);

INTERNAL VOID
VmbusOnEvents(
  VOID
	);


#endif  /* __HV_VMBUS_PRIVATE_H__ */

