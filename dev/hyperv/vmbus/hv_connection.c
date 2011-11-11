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
 * HyperV vmbus connection functionality
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

#ifdef REMOVED
/* Fixme:  Removed */
#include "logging.h"

#include "VmbusPrivate.h"
#endif

#include <dev/hyperv/include/hv_osd.h>
#include <dev/hyperv/include/hv_logging.h>
/* Fixme -- contains globals, cannot be included more than once */
//#include "hv_version_info.h"
#include "hv_hv.h"
#include "hv_vmbus_var.h"
#include "hv_vmbus_api.h"
#include <dev/hyperv/include/hv_list.h>
#include "hv_ring_buffer.h"
#include <dev/hyperv/include/hv_vmbus_channel_interface.h>
#include <dev/hyperv/include/hv_vmbus_packet_format.h>
#include <dev/hyperv/include/hv_channel_messages.h>
#include "hv_channel_mgmt.h"
#include "hv_channel.h"
#include "hv_channel_interface.h"
// Fixme:  need this?  Was in hv_vmbus_private.h
//#include "timesync_ic.h"
#include "hv_vmbus_private.h"
#include "hv_connection.h"


//
// Globals
//


VMBUS_CONNECTION gVmbusConnection = {
	.ConnectState		= Disconnected,
	.NextGpadlHandle	= 0xE1E10, 
};


/*++

Name: 
	VmbusConnect()

Description:
	Sends a connect request on the partition service connection

--*/
int
VmbusConnect(void)
{
	int ret = 0;
	VMBUS_CHANNEL_MSGINFO *msgInfo = NULL;
	VMBUS_CHANNEL_INITIATE_CONTACT *msg;

	DPRINT_ENTER(VMBUS);

	// Make sure we are not connecting or connected
	if (gVmbusConnection.ConnectState != Disconnected) {
		return -1;
	}

	// Initialize the vmbus connection
	gVmbusConnection.ConnectState = Connecting;
	gVmbusConnection.WorkQueue = WorkQueueCreate("vmbusQ");

	INITIALIZE_LIST_HEAD(&gVmbusConnection.ChannelMsgList);
	gVmbusConnection.ChannelMsgLock = SpinlockCreate();
	
	INITIALIZE_LIST_HEAD(&gVmbusConnection.ChannelList);
	gVmbusConnection.ChannelLock = SpinlockCreate();

	// Set up the vmbus event connection for channel interrupt abstraction
	// stuff
	gVmbusConnection.InterruptPage = PageAlloc(1);
	if (gVmbusConnection.InterruptPage == NULL)
	{
		ret = -1;
		goto Cleanup;
	}

	gVmbusConnection.RecvInterruptPage = gVmbusConnection.InterruptPage;
	gVmbusConnection.SendInterruptPage =
	    (void*)((ULONG_PTR)gVmbusConnection.InterruptPage + (PAGE_SIZE >> 1));

	// Set up the monitor notification facility. The 1st page for
	// parent->child and the 2nd page for child->parent
	gVmbusConnection.MonitorPages = PageAlloc(2);
	if (gVmbusConnection.MonitorPages == NULL)
	{
		ret = -1;
		goto Cleanup;
	}

	msgInfo = (VMBUS_CHANNEL_MSGINFO*)MemAllocZeroed(
	    sizeof(VMBUS_CHANNEL_MSGINFO) + 
	    sizeof(VMBUS_CHANNEL_INITIATE_CONTACT));
	if (msgInfo == NULL)
	{
		ret = -1;
		goto Cleanup;
	}

	msgInfo->WaitEvent = WaitEventCreate();
	msg = (VMBUS_CHANNEL_INITIATE_CONTACT*)msgInfo->Msg;

	msg->Header.MessageType = ChannelMessageInitiateContact;
	msg->VMBusVersionRequested = VMBUS_REVISION_NUMBER;
	msg->InterruptPage = GetPhysicalAddress(gVmbusConnection.InterruptPage);
	msg->MonitorPage1 = GetPhysicalAddress(gVmbusConnection.MonitorPages);
	msg->MonitorPage2 = GetPhysicalAddress(
	    (PVOID)((ULONG_PTR)gVmbusConnection.MonitorPages + PAGE_SIZE));
	
	// Add to list before we send the request since we may receive the
	// response before returning from this routine
	SpinlockAcquire(gVmbusConnection.ChannelMsgLock);
	INSERT_TAIL_LIST(&gVmbusConnection.ChannelMsgList, &msgInfo->MsgListEntry);
	SpinlockRelease(gVmbusConnection.ChannelMsgLock);

	DPRINT_DBG(VMBUS, "Vmbus connection:  interrupt pfn %llx, monitor1 pfn "
		"%llx,, monitor2 pfn %llx", 
		msg->InterruptPage, msg->MonitorPage1, msg->MonitorPage2);

	DPRINT_DBG(VMBUS, "Sending channel initiate msg...");

	ret = VmbusPostMessage(msg, sizeof(VMBUS_CHANNEL_INITIATE_CONTACT));
	if (ret != 0)
	{
		REMOVE_ENTRY_LIST(&msgInfo->MsgListEntry);
		goto Cleanup;
	}
	
	// Wait for the connection response
	/* Fixme:  Error message added */
	DPRINT_DBG(VMBUS, "Wait for the Connection response ...");
	WaitEventWait(msgInfo->WaitEvent);
	/* Fixme:  Error message added */
	DPRINT_DBG(VMBUS, "Got the Connection response ...");

	REMOVE_ENTRY_LIST(&msgInfo->MsgListEntry);

	// Check if successful
	if (msgInfo->Response.VersionResponse.VersionSupported)
	{
		DPRINT_INFO(VMBUS, "Vmbus connected!!"); 
		gVmbusConnection.ConnectState = Connected;
	}
	else
	{
		DPRINT_ERR(VMBUS, "Vmbus connection failed!!...current version"
		    " (%d) not supported", VMBUS_REVISION_NUMBER); 
		ret = -1;

		goto Cleanup;
	}

	WaitEventClose(msgInfo->WaitEvent);
	MemFree(msgInfo);
	DPRINT_EXIT(VMBUS);

	return 0;

Cleanup:

	gVmbusConnection.ConnectState = Disconnected;

	WorkQueueClose(gVmbusConnection.WorkQueue);
	SpinlockClose(gVmbusConnection.ChannelLock);
	SpinlockClose(gVmbusConnection.ChannelMsgLock);

	if (gVmbusConnection.InterruptPage)
	{
		PageFree(gVmbusConnection.InterruptPage, 1);
		gVmbusConnection.InterruptPage = NULL;
	}

	if (gVmbusConnection.MonitorPages)
	{
		PageFree(gVmbusConnection.MonitorPages, 2);
		gVmbusConnection.MonitorPages = NULL;
	}

	if (msgInfo)
	{
		if (msgInfo->WaitEvent)
			WaitEventClose(msgInfo->WaitEvent);

		MemFree(msgInfo);
	}
	
	DPRINT_EXIT(VMBUS);

	return ret;
}


/*++

Name: 
	VmbusDisconnect()

Description:
	Sends a disconnect request on the partition service connection

--*/
int
VmbusDisconnect(
	VOID
	)
{
	int ret=0;
	VMBUS_CHANNEL_UNLOAD *msg;

	DPRINT_ENTER(VMBUS);

	// Make sure we are connected
	if (gVmbusConnection.ConnectState != Connected)
		return -1;

	msg = MemAllocZeroed(sizeof(VMBUS_CHANNEL_UNLOAD));

	msg->MessageType = ChannelMessageUnload;

	ret = VmbusPostMessage(msg, sizeof(VMBUS_CHANNEL_UNLOAD));

	if (ret != 0)
	{
		goto Cleanup;
	}

	PageFree(gVmbusConnection.InterruptPage, 1);

	// TODO: iterate thru the msg list and free up

	SpinlockClose(gVmbusConnection.ChannelMsgLock);

	WorkQueueClose(gVmbusConnection.WorkQueue);

	gVmbusConnection.ConnectState = Disconnected;

	DPRINT_INFO(VMBUS, "Vmbus disconnected!!"); 

Cleanup:
	if (msg)
	{
		MemFree(msg);
	}

	DPRINT_EXIT(VMBUS);

	return ret;
}


/*++

Name: 
	GetChannelFromRelId()

Description:
	Get the channel object given its child relative id (ie channel id)
	
--*/
VMBUS_CHANNEL*
GetChannelFromRelId(
	UINT32 relId
	)
{
	VMBUS_CHANNEL* channel;
	VMBUS_CHANNEL* foundChannel=NULL;
	LIST_ENTRY* anchor;
	LIST_ENTRY* curr;

	SpinlockAcquire(gVmbusConnection.ChannelLock);
	ITERATE_LIST_ENTRIES(anchor, curr, &gVmbusConnection.ChannelList)
	{		
		channel = CONTAINING_RECORD(curr, VMBUS_CHANNEL, ListEntry);

		if (channel->OfferMsg.ChildRelId == relId)
		{
			foundChannel = channel;
			break;
		}
	}
	SpinlockRelease(gVmbusConnection.ChannelLock);

	return foundChannel;
}



/*++

Name: 
	VmbusProcessChannelEvent()

Description:
	Process a channel event notification

--*/
static void 
VmbusProcessChannelEvent(
	PVOID context
	)
{
	VMBUS_CHANNEL* channel;
	UINT32 relId = (UINT32)(ULONG_PTR)context;

	ASSERT(relId > 0);
	/* Fixme:  Added for NetScaler */
	/* If the channel is in poll mode, return */
	if (VmbusGetChannelMode(relId)) {
		return;
	}

	// Find the channel based on this relid and invokes
	// the channel callback to process the event
	channel = GetChannelFromRelId(relId);

	if (channel)
	{
		VmbusChannelOnChannelEvent(channel);
		//WorkQueueQueueWorkItem(channel->dataWorkQueue, VmbusChannelOnChannelEvent, (void*)channel);
	}
	else
	{
        DPRINT_ERR(VMBUS, "channel not found for relid - %d.", relId);
	}
}


/*++

Name: 
	VmbusOnEvents()

Description:
	Handler for events

--*/
VOID
VmbusOnEvents(
  VOID
	)
{
	int dword;
	//int maxdword = PAGE_SIZE >> 3; // receive size is 1/2 page and divide that by 4 bytes
	int maxdword = MAX_NUM_CHANNELS_SUPPORTED >> 5;
	int bit;
	int relid;
	/* Fixme:  not in lis21 code */
	int mycnt = 0;
	/* Fixme:  not in lis21 code */
	int xcnt = 0;
	UINT32* recvInterruptPage = gVmbusConnection.RecvInterruptPage;
	//VMBUS_CHANNEL_MESSAGE* receiveMsg;

	DPRINT_ENTER(VMBUS);

	// Check events
	if (recvInterruptPage)
	{
		for (dword = 0; dword < maxdword; dword++)
		{
			if (recvInterruptPage[dword])
			{
				xcnt++;
				for (bit = 0; bit < 32; bit++)
				{
					if (BitTestAndClear(&recvInterruptPage[dword], bit))
					{
						relid = (dword << 5) + bit;
						mycnt++;

						/* Fixme:  Commented out */
//						DPRINT_DBG(VMBUS, "event detected for relid - %d", relid);

						if (relid == 0) // special case - vmbus channel protocol msg
						{
							DPRINT_DBG(VMBUS, "invalid relid - %d", relid);

							continue;						}
						else
						{
							//QueueWorkItem(VmbusProcessEvent, (void*)relid);
							//ret = WorkQueueQueueWorkItem(gVmbusConnection.workQueue, VmbusProcessChannelEvent, (void*)relid);
							VmbusProcessChannelEvent((void*)(ULONG_PTR)relid);
						}
					}
				}
			}
		 }
	}
	/* Fixme:  not in lis21 code */
	if (mycnt == 0) {
//		printf("No event to process - BUG: %x\n", xcnt);
	}
	DPRINT_EXIT(VMBUS);

	return;
}

/*++

Name: 
	VmbusPostMessage()

Description:
	Send a msg on the vmbus's message connection

--*/
int
VmbusPostMessage(
	PVOID			buffer,
	SIZE_T			bufferLen
	)
{
	int ret=0;
	HV_CONNECTION_ID connId;


	connId.AsUINT32 =0;
	connId.u.Id = VMBUS_MESSAGE_CONNECTION_ID;
	ret = HvPostMessage(
			connId,
			1,
			buffer,
			bufferLen);

	return  ret;
}

/*++

Name: 
	VmbusSetEvent()

Description:
	Send an event notification to the parent

--*/
int
VmbusSetEvent(UINT32 childRelId)
{
	int ret=0;
	
	DPRINT_ENTER(VMBUS);

	// Each UINT32 represents 32 channels
	BitSet((UINT32*)gVmbusConnection.SendInterruptPage + (childRelId >> 5), childRelId & 31);
	ret = HvSignalEvent();

	DPRINT_EXIT(VMBUS);

	return ret;
}
          
/*
 * Fixme:  NetScaler.  The functions below were added for NetScaler,
 * determine if they are needed for the FreeBSD porting effort.
 */

/*++

Name: 
	VmbusSetChannelMode()

Description:
	Set the Channel mode to either poll or Interrupt
	mode 1: Poll
	mode 0: Interrupt
--*/

static int VmbusChannelMode[MAX_NUM_CHANNELS_SUPPORTED >> 5];

void
VmbusSetChannelMode(PVOID context, int mode)
{
	VMBUS_CHANNEL *channel = (VMBUS_CHANNEL *)context;
	UINT32 relId = channel->OfferMsg.ChildRelId;
	UINT32 bit = (1U << (relId & 0x1f));

	if (mode) {
		VmbusChannelMode[relId >> 5] |= bit;
		ClearRingBufferInterruptMask(&channel->Inbound);
	} else {
		VmbusChannelMode[relId >> 5] &= ~bit;
		SetRingBufferInterruptMask(&channel->Inbound);
	}

	return;
}

int
VmbusGetChannelMode(UINT32 relId)
{
	UINT32 bit = (1U << (relId & 0x1f));

	if (relId > MAX_NUM_CHANNELS_SUPPORTED)
		return 0;

	return((VmbusChannelMode[relId >> 5] & bit) ? 1 : 0);
}

int CheckEvents(void)
{
	UINT32* recvInterruptPage = gVmbusConnection.RecvInterruptPage;
	int maxdword = MAX_NUM_CHANNELS_SUPPORTED >> 5;
	int dword;

	if (recvInterruptPage) {
		for (dword = 0; dword < maxdword; dword++) {
            		if (recvInterruptPage[dword]) {
				/* clear the interrupts */
				recvInterruptPage[dword] &= ~VmbusChannelMode[dword];
				if (recvInterruptPage[dword])
					return 1;
			}
		}
	}

	return 0;

}

int VmbusDataReady(PVOID context)
{
	VMBUS_CHANNEL *channel = (VMBUS_CHANNEL *)context;
	return RingBufferCheck(&channel->Inbound);
}

// EOF
