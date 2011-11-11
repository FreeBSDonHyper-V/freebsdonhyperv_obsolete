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
 * Public wrapper around the internal channel APIs
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
/* Fixme -- removed */
#include "VmbusPrivate.h"
#endif

#ifdef REMOVED
#include "Hv.h"
#include "VmbusApi.h"
#include "Channel.h"
#include "ChannelMgmt.h"
  #include "VmbusApi.h"
#include "ChannelInterface.h"
#include "ChannelMessages.h"
#include "RingBuffer.h"
//#include "Packet.h"
#include "List.h"
#include "timesync_ic.h"
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


/*
 * Fixme:  This kludge needed due to INTERNAL definition kludge in header
 */
#ifdef INTERNAL
#undef INTERNAL
#define INTERNAL static
#endif


INTERNAL int	
IVmbusChannelOpen(
	PDEVICE_OBJECT		Device,
	UINT32				SendBufferSize,
	UINT32				RecvRingBufferSize,
	PVOID				UserData,
	UINT32				UserDataLen,
	VMBUS_CHANNEL_CALLBACK ChannelCallback,
	PVOID				Context
	)
{
	return VmbusChannelOpen( (VMBUS_CHANNEL*)Device->context,
								SendBufferSize,
								RecvRingBufferSize,
								UserData,
								UserDataLen,
								ChannelCallback,
								Context);
}


INTERNAL void
IVmbusChannelClose(
	PDEVICE_OBJECT		Device
	)
{
	VmbusChannelClose((VMBUS_CHANNEL*)Device->context);
}


INTERNAL int
IVmbusChannelSendPacket(
	PDEVICE_OBJECT		Device, 
	const PVOID			Buffer, 
	UINT32				BufferLen,
	UINT64				RequestId,
	UINT32				Type,
	UINT32				Flags
	)
{
	return VmbusChannelSendPacket((VMBUS_CHANNEL*)Device->context,
									Buffer,
									BufferLen,
									RequestId,
									Type,
									Flags);
}

INTERNAL int
IVmbusChannelSendPacketPageBuffer(
	PDEVICE_OBJECT		Device,
	PAGE_BUFFER			PageBuffers[],
	UINT32				PageCount,
	PVOID				Buffer,
	UINT32				BufferLen,
	UINT64				RequestId
	)
{
	return VmbusChannelSendPacketPageBuffer((VMBUS_CHANNEL*)Device->context,
												PageBuffers,
												PageCount,
												Buffer,
												BufferLen,
												RequestId);
}

INTERNAL int
IVmbusChannelSendPacketMultiPageBuffer(
	PDEVICE_OBJECT		Device,
	MULTIPAGE_BUFFER	*MultiPageBuffer,
	PVOID				Buffer,
	UINT32				BufferLen,
	UINT64				RequestId
	)
{
	return VmbusChannelSendPacketMultiPageBuffer((VMBUS_CHANNEL*)Device->context,
													MultiPageBuffer,
													Buffer,
													BufferLen,
													RequestId);
}

INTERNAL int
IVmbusChannelRecvPacket (
	PDEVICE_OBJECT		Device,
	PVOID				Buffer,
	UINT32				BufferLen,
	UINT32*				BufferActualLen,
	UINT64*				RequestId
	)
{
	return VmbusChannelRecvPacket((VMBUS_CHANNEL*)Device->context,
									Buffer,
									BufferLen,
									BufferActualLen,
									RequestId);
}

INTERNAL int
IVmbusChannelRecvPacketRaw(
	PDEVICE_OBJECT		Device,
	PVOID				Buffer,
	UINT32				BufferLen,
	UINT32*				BufferActualLen,
	UINT64*				RequestId
	)
{
	return VmbusChannelRecvPacketRaw((VMBUS_CHANNEL*)Device->context,
										Buffer,
										BufferLen,
										BufferActualLen,
										RequestId);
}

INTERNAL int
IVmbusChannelEstablishGpadl(
	PDEVICE_OBJECT		Device,
	PVOID				Buffer,
	UINT32				BufferLen,
	UINT32*				GpadlHandle
	)
{
	return VmbusChannelEstablishGpadl((VMBUS_CHANNEL*)Device->context,
										Buffer,
										BufferLen,
										GpadlHandle);
}

INTERNAL int
IVmbusChannelTeardownGpadl(
   PDEVICE_OBJECT		Device,
   UINT32				GpadlHandle
	)
{
	return VmbusChannelTeardownGpadl((VMBUS_CHANNEL*)Device->context,
										GpadlHandle);

}

/*
 * Fixme:  This kludge needed due to INTERNAL definition kludge above
 */
#ifdef INTERNAL
#undef INTERNAL
#define INTERNAL extern
#endif



INTERNAL void
GetChannelInterface(
	VMBUS_CHANNEL_INTERFACE *ChannelInterface
	)
{
	ChannelInterface->Open						= IVmbusChannelOpen;
	ChannelInterface->Close						= IVmbusChannelClose;
	ChannelInterface->SendPacket				= IVmbusChannelSendPacket;
	ChannelInterface->SendPacketPageBuffer		= IVmbusChannelSendPacketPageBuffer;
	ChannelInterface->SendPacketMultiPageBuffer = IVmbusChannelSendPacketMultiPageBuffer;
	ChannelInterface->RecvPacket				= IVmbusChannelRecvPacket;
	ChannelInterface->RecvPacketRaw				= IVmbusChannelRecvPacketRaw;
	ChannelInterface->EstablishGpadl			= IVmbusChannelEstablishGpadl;
	ChannelInterface->TeardownGpadl				= IVmbusChannelTeardownGpadl;
	ChannelInterface->GetInfo					= GetChannelInfo;
}


INTERNAL void
GetChannelInfo(
	PDEVICE_OBJECT		Device,
	DEVICE_INFO			*DeviceInfo
			   )
{
	VMBUS_CHANNEL_DEBUG_INFO debugInfo;

	if (Device->context)
	{
		VmbusChannelGetDebugInfo((VMBUS_CHANNEL*)Device->context, &debugInfo);

		DeviceInfo->ChannelId = debugInfo.RelId;
		DeviceInfo->ChannelState = debugInfo.State;
		memcpy(&DeviceInfo->ChannelType, &debugInfo.InterfaceType, sizeof(GUID));
		memcpy(&DeviceInfo->ChannelInstance, &debugInfo.InterfaceInstance, sizeof(GUID));

		DeviceInfo->MonitorId = debugInfo.MonitorId;

		DeviceInfo->ServerMonitorPending = debugInfo.ServerMonitorPending;
		DeviceInfo->ServerMonitorLatency = debugInfo.ServerMonitorLatency;
		DeviceInfo->ServerMonitorConnectionId = debugInfo.ServerMonitorConnectionId;

		DeviceInfo->ClientMonitorPending = debugInfo.ClientMonitorPending;
		DeviceInfo->ClientMonitorLatency = debugInfo.ClientMonitorLatency;
		DeviceInfo->ClientMonitorConnectionId = debugInfo.ClientMonitorConnectionId;

		DeviceInfo->Inbound.InterruptMask = debugInfo.Inbound.CurrentInterruptMask;
		DeviceInfo->Inbound.ReadIndex = debugInfo.Inbound.CurrentReadIndex;
		DeviceInfo->Inbound.WriteIndex = debugInfo.Inbound.CurrentWriteIndex;
		DeviceInfo->Inbound.BytesAvailToRead = debugInfo.Inbound.BytesAvailToRead;
		DeviceInfo->Inbound.BytesAvailToWrite = debugInfo.Inbound.BytesAvailToWrite;

		DeviceInfo->Outbound.InterruptMask = debugInfo.Outbound.CurrentInterruptMask;
		DeviceInfo->Outbound.ReadIndex = debugInfo.Outbound.CurrentReadIndex;
		DeviceInfo->Outbound.WriteIndex = debugInfo.Outbound.CurrentWriteIndex;
		DeviceInfo->Outbound.BytesAvailToRead = debugInfo.Outbound.BytesAvailToRead;
		DeviceInfo->Outbound.BytesAvailToWrite = debugInfo.Outbound.BytesAvailToWrite;
	}
}
