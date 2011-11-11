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
 * Channel definition file
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

#ifndef __HV_CHANNEL_H__
#define __HV_CHANNEL_H__

#ifdef REMOVED
/* Fixme -- removed */
#include "osd.h"
#include "ChannelMgmt.h"
#endif

#pragma pack(push,1)


// The format must be the same as VMDATA_GPA_DIRECT
typedef struct _VMBUS_CHANNEL_PACKET_PAGE_BUFFER {
    UINT16				Type;		
    UINT16				DataOffset8;
    UINT16				Length8;	
    UINT16				Flags;
    UINT64				TransactionId;
	UINT32				Reserved;
	UINT32				RangeCount;
    PAGE_BUFFER			Range[MAX_PAGE_BUFFER_COUNT];
} VMBUS_CHANNEL_PACKET_PAGE_BUFFER;


// The format must be the same as VMDATA_GPA_DIRECT
typedef struct _VMBUS_CHANNEL_PACKET_MULITPAGE_BUFFER {
    UINT16				Type;		
    UINT16				DataOffset8;
    UINT16				Length8;	
    UINT16				Flags;
    UINT64				TransactionId;
	UINT32				Reserved;
	UINT32				RangeCount;		// Always 1 in this case
	MULTIPAGE_BUFFER	Range;
} VMBUS_CHANNEL_PACKET_MULITPAGE_BUFFER;

#pragma pack(pop)

//
// Routines
//

INTERNAL int
VmbusChannelOpen(
	VMBUS_CHANNEL			*Channel,
	UINT32					SendRingBufferSize,
	UINT32					RecvRingBufferSize,
	PVOID					UserData,
	UINT32					UserDataLen,
	PFN_CHANNEL_CALLBACK	pfnOnChannelCallback,
	PVOID					Context
	);

INTERNAL void
VmbusChannelClose(
	VMBUS_CHANNEL		*Channel
	);

INTERNAL int
VmbusChannelSendPacket(
	VMBUS_CHANNEL		*Channel,
	const PVOID			Buffer,
	UINT32				BufferLen,
	UINT64				RequestId,
	VMBUS_PACKET_TYPE	Type,
	UINT32				Flags
);

INTERNAL int
VmbusChannelSendPacketPageBuffer(
	VMBUS_CHANNEL		*Channel,
	PAGE_BUFFER			PageBuffers[],
	UINT32				PageCount,
	PVOID				Buffer,
	UINT32				BufferLen,
	UINT64				RequestId
	);

INTERNAL int
VmbusChannelSendPacketMultiPageBuffer(
	VMBUS_CHANNEL		*Channel,
	MULTIPAGE_BUFFER	*MultiPageBuffer,
	PVOID				Buffer,
	UINT32				BufferLen,
	UINT64				RequestId
);

INTERNAL int
VmbusChannelEstablishGpadl(
	VMBUS_CHANNEL		*Channel,
	PVOID				Kbuffer,	// from kmalloc()
	UINT32				Size,		// page-size multiple
	UINT32				*GpadlHandle
	);

INTERNAL int
VmbusChannelTeardownGpadl(
	VMBUS_CHANNEL	*Channel,
	UINT32			GpadlHandle
	);

INTERNAL int
VmbusChannelRecvPacket(
	VMBUS_CHANNEL		*Channel,
	PVOID				Buffer,
	UINT32				BufferLen,
	UINT32*				BufferActualLen,
	UINT64*				RequestId
	);

INTERNAL int
VmbusChannelRecvPacketRaw(
	VMBUS_CHANNEL		*Channel,
	PVOID				Buffer,
	UINT32				BufferLen,
	UINT32*				BufferActualLen,
	UINT64*				RequestId
	);

INTERNAL void
VmbusChannelOnChannelEvent(
	VMBUS_CHANNEL		*Channel
	);

INTERNAL void
VmbusChannelGetDebugInfo(
	VMBUS_CHANNEL				*Channel,
	VMBUS_CHANNEL_DEBUG_INFO	*DebugInfo
	);

INTERNAL void
VmbusChannelOnTimer(
	void		*Context
	);

#endif  /* __HV_CHANNEL_H__ */

