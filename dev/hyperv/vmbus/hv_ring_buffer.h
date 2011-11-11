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
 * HyperV ring buffer definition file
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

#ifndef __HV_RING_BUFFER_H__
#define __HV_RING_BUFFER_H__

#ifdef REMOVED
/* Fixme:  Removed */
#include "osd.h"
#endif

typedef struct _SG_BUFFER_LIST {
	PVOID	Data;
	UINT32	Length;
} SG_BUFFER_LIST;

typedef struct _RING_BUFFER {
    volatile UINT32	WriteIndex;     // Offset in bytes from the start of ring data below
    volatile UINT32	ReadIndex;      // Offset in bytes from the start of ring data below

	volatile UINT32 InterruptMask;
	UINT8	Reserved[4084];			// Pad it to PAGE_SIZE so that data starts on page boundary
	// NOTE: The InterruptMask field is used only for channels but since our vmbus connection
	// also uses this data structure and its data starts here, we commented out this field.
	// volatile UINT32 InterruptMask;
	// Ring data starts here + RingDataStartOffset !!! DO NOT place any fields below this !!!
    UINT8		Buffer[0];
} STRUCT_PACKED RING_BUFFER;

typedef struct _RING_BUFFER_INFO {
    RING_BUFFER*	RingBuffer;
    UINT32			RingSize;			// Include the shared header
	HANDLE			RingLock;

    UINT32			RingDataSize;		// < ringSize
	UINT32			RingDataStartOffset;

} RING_BUFFER_INFO;


typedef struct _RING_BUFFER_DEBUG_INFO {
	UINT32		CurrentInterruptMask;
	UINT32		CurrentReadIndex;
	UINT32		CurrentWriteIndex;
	UINT32		BytesAvailToRead;
	UINT32		BytesAvailToWrite;
}RING_BUFFER_DEBUG_INFO;


//
// Interface
//

INTERNAL int
RingBufferInit(
	RING_BUFFER_INFO	*RingInfo,
	PVOID				Buffer,
	UINT32				BufferLen
	);

INTERNAL void
RingBufferCleanup(
	RING_BUFFER_INFO	*RingInfo
	);

INTERNAL int
RingBufferWrite(
	RING_BUFFER_INFO	*RingInfo,
	SG_BUFFER_LIST		SgBuffers[],
	UINT32				SgBufferCount
	);

INTERNAL int
RingBufferPeek(
	RING_BUFFER_INFO	*RingInfo,
	PVOID				Buffer,
	UINT32				BufferLen
	);

INTERNAL int
RingBufferRead(
	RING_BUFFER_INFO	*RingInfo,
	PVOID				Buffer,
	UINT32				BufferLen,
	UINT32				Offset
	);

INTERNAL UINT32 
GetRingBufferInterruptMask(
	RING_BUFFER_INFO *RingInfo
	);

INTERNAL void
DumpRingInfo(
	RING_BUFFER_INFO* RingInfo, 
	char *Prefix
	);

INTERNAL void
RingBufferGetDebugInfo(
	RING_BUFFER_INFO		*RingInfo,
	RING_BUFFER_DEBUG_INFO	*DebugInfo
	);


/*
 * Externs
 */
extern void SetRingBufferInterruptMask(RING_BUFFER_INFO *rbi);
extern void ClearRingBufferInterruptMask(RING_BUFFER_INFO *rbi);
extern int  RingBufferCheck(RING_BUFFER_INFO *rbi);


#endif  /* __HV_RING_BUFFER_H__ */

