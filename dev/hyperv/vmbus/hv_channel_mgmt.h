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
 * Ported from lis21 code drop
 *
 * Channel management definition file
 *
 *****************************************************************************/

/*
 * Copyright (c) 2009, Microsoft Corporation - All rights reserved.
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      - Redistributions of source code must retain the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer.
 *
 *      - Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following
 *        disclaimer in the documentation and/or other materials
 *        provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *   Haiyang Zhang <haiyangz@microsoft.com>
 *   Hank Janssen  <hjanssen@microsoft.com>
 */

#ifndef __HV_CHANNEL_MGMT_H__
#define __HV_CHANNEL_MGMT_H__

#ifdef REMOVED
/* Fixme -- removed */
#include "osd.h"
#include "List.h"
#include "RingBuffer.h"

#include "VmbusChannelInterface.h"
#include "ChannelMessages.h"
#endif



typedef void (*PFN_CHANNEL_CALLBACK)(PVOID context);

typedef enum {
	CHANNEL_OFFER_STATE,
	CHANNEL_OPENING_STATE,
	CHANNEL_OPEN_STATE,
} VMBUS_CHANNEL_STATE;

typedef struct _VMBUS_CHANNEL {
	LIST_ENTRY					ListEntry;

	DEVICE_OBJECT*				DeviceObject;

	HANDLE						PollTimer; // SA-111 workaround

	VMBUS_CHANNEL_STATE			State;

	VMBUS_CHANNEL_OFFER_CHANNEL OfferMsg;
	// These are based on the OfferMsg.MonitorId. Save it here for easy access.
	UINT8						MonitorGroup;
	UINT8						MonitorBit;

	UINT32						RingBufferGpadlHandle;

	// Allocated memory for ring buffer
	VOID*						RingBufferPages;
	UINT32						RingBufferPageCount;
	RING_BUFFER_INFO			Outbound;	// send to parent
	RING_BUFFER_INFO			Inbound;	// receive from parent
	HANDLE						InboundLock;
	HANDLE						ControlWQ;
		
	// Channel callback are invoked in this workqueue context
	//HANDLE						dataWorkQueue;

	PFN_CHANNEL_CALLBACK		OnChannelCallback;
	PVOID						ChannelCallbackContext;

} VMBUS_CHANNEL;


typedef struct _VMBUS_CHANNEL_DEBUG_INFO {
	UINT32						RelId;
	VMBUS_CHANNEL_STATE			State;
	GUID						InterfaceType;
    GUID						InterfaceInstance;
	UINT32						MonitorId;
	UINT32						ServerMonitorPending;
	UINT32						ServerMonitorLatency;
	UINT32						ServerMonitorConnectionId;
	UINT32						ClientMonitorPending;
	UINT32						ClientMonitorLatency;
	UINT32						ClientMonitorConnectionId;

	RING_BUFFER_DEBUG_INFO		Inbound;
	RING_BUFFER_DEBUG_INFO		Outbound;
} VMBUS_CHANNEL_DEBUG_INFO;


typedef union {
	VMBUS_CHANNEL_VERSION_SUPPORTED		VersionSupported;
	VMBUS_CHANNEL_OPEN_RESULT			OpenResult;
	VMBUS_CHANNEL_GPADL_TORNDOWN		GpadlTorndown;
	VMBUS_CHANNEL_GPADL_CREATED			GpadlCreated;
	VMBUS_CHANNEL_VERSION_RESPONSE		VersionResponse;
} VMBUS_CHANNEL_MESSAGE_RESPONSE;


// Represents each channel msg on the vmbus connection
// This is a variable-size data structure depending on
// the msg type itself
typedef struct _VMBUS_CHANNEL_MSGINFO {
	// Bookkeeping stuff
	LIST_ENTRY		MsgListEntry;

	// So far, this is only used to handle gpadl body message
	LIST_ENTRY		SubMsgList;

	// Synchronize the request/response if needed
	HANDLE			WaitEvent;

	VMBUS_CHANNEL_MESSAGE_RESPONSE Response;

	UINT32			MessageSize;
	// The channel message that goes out on the "wire".
	// It will contain at minimum the VMBUS_CHANNEL_MESSAGE_HEADER header
	unsigned char	Msg[0];
} VMBUS_CHANNEL_MSGINFO;


//
// Routines
//

INTERNAL VMBUS_CHANNEL* 
AllocVmbusChannel(
	void
	);

INTERNAL void
FreeVmbusChannel(
	VMBUS_CHANNEL *Channel
	);

INTERNAL void
VmbusOnChannelMessage(
	void *Context
	);

INTERNAL int
VmbusChannelRequestOffers(
	void
	);

INTERNAL void
VmbusChannelReleaseUnattachedChannels(
	void
	);

#endif  /* __HV_CHANNEL_MGMT_H__ */

