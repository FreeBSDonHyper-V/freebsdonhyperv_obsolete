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
 * HyperV structures that define the channel management interfaces
 * exported by the VMBus driver.
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

#ifndef __HV_VMBUS_CHANNEL_INTERFACE_H__
#define __HV_VMBUS_CHANNEL_INTERFACE_H__


#pragma once
// allow nameless unions
//#pragma warning(disable : 4201)

// 
// A revision number of vmbus that is used for ensuring both ends on a
// partition are using compatible versions.
//
#define VMBUS_REVISION_NUMBER       13

//
// Make maximum size of pipe payload of 16K
//
#define MAX_PIPE_DATA_PAYLOAD 		(sizeof(BYTE) * 16384)

//
// Define PipeMode values.
//
#define VMBUS_PIPE_TYPE_BYTE                    0x00000000
#define VMBUS_PIPE_TYPE_MESSAGE                 0x00000004

//
// The size of the user defined data buffer for non-pipe offers.
//
#define MAX_USER_DEFINED_BYTES                  120

//
// The size of the user defined data buffer for pipe offers.
//
#define MAX_PIPE_USER_DEFINED_BYTES             116


//
// At the center of the Channel Management library is
// the Channel Offer. This struct contains the
// fundamental information about an offer.
//
#pragma pack(push,1)

typedef struct
{

    GUID    InterfaceType;
    GUID    InterfaceInstance;
    UINT64  InterruptLatencyIn100nsUnits;
    UINT32  InterfaceRevision;
    UINT32  ServerContextAreaSize;  // in bytes
    UINT16  ChannelFlags;
    UINT16  MmioMegabytes;          // in bytes * 1024 * 1024

    union
    {
        //
        // Non-pipes: The user has MAX_USER_DEFINED_BYTES bytes.
        //
        struct
        {
            UCHAR   UserDefined[MAX_USER_DEFINED_BYTES];
        } Standard;

        //
        // Pipes: The following sructure is an integrated pipe protocol, which
        //        is implemented on top of standard user-defined data. Pipe clients
        //        have MAX_PIPE_USER_DEFINED_BYTES left for their own use.
        //
        struct
        {
            UINT32  PipeMode;
            UCHAR   UserDefined[MAX_PIPE_USER_DEFINED_BYTES];
        } Pipe;
    } u;
	UINT32	Padding;
} VMBUS_CHANNEL_OFFER, *PVMBUS_CHANNEL_OFFER;
#pragma pack(pop)


//
// Verify the MAX_PIPE_USER_DEFINED_BYTES value.
//
//C_ASSERT(MAX_PIPE_USER_DEFINED_BYTES == 
//         MAX_USER_DEFINED_BYTES - 
//         (FIELD_OFFSET(VMBUS_CHANNEL_OFFER, u.Pipe.UserDefined) - 
//          FIELD_OFFSET(VMBUS_CHANNEL_OFFER, u.Standard.UserDefined)));
//

typedef UINT32 GPADL_HANDLE;

//
// Server Flags
//

#define VMBUS_CHANNEL_ENUMERATE_DEVICE_INTERFACE           1
#define VMBUS_CHANNEL_SERVER_SUPPORTS_TRANSFER_PAGES       2
#define VMBUS_CHANNEL_SERVER_SUPPORTS_GPADLS               4
#define VMBUS_CHANNEL_NAMED_PIPE_MODE                   0x10
#define VMBUS_CHANNEL_LOOPBACK_OFFER                   0x100
#define VMBUS_CHANNEL_PARENT_OFFER                     0x200
#define VMBUS_CHANNEL_REQUEST_MONITORED_NOTIFICATION   0x400

//
// TEMPTEMP -- move this next define to devioctl.h some day
//

#ifndef FILE_DEVICE_VMBUS   
#define FILE_DEVICE_VMBUS   0x0000003E
#endif

#endif  /* __HV_VMBUS_CHANNEL_INTERFACE_H__ */

