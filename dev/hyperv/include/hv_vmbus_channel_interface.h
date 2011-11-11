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
 * HyperV structures that define the channel management interfaces
 * exported by the VMBus driver.
 *
 *****************************************************************************/

/*----------------------------------------------------------------------------
 $Microsoft Confidential$
 $Copyright (C) 2004 Microsoft Corporation.  All Rights Reserved.$

 $File: VmbusChannelInterface.w $

 Abstract:

     This file contains the structures that define the channel management
     interfaces exported by the VMBus driver.

----------------------------------------------------------------------------*/

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

#ifdef _MSFT_
enum _VMBUS_CHANNEL_IOCTL_TYPE
{
    //
    // Shared IOCTLs
    //

    IOCTL_VMBUS_WAIT_FOR_ACTION  = CTL_CODE(FILE_DEVICE_VMBUS,  0x2, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS),
    IOCTL_VMBUS_MAP_GPADL_VIEW   = CTL_CODE(FILE_DEVICE_VMBUS,  0x3, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS),
    IOCTL_VMBUS_UNMAP_GPADL_VIEW = CTL_CODE(FILE_DEVICE_VMBUS,  0x4, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS),
    IOCTL_VMBUS_SEND_INTERRUPT   = CTL_CODE(FILE_DEVICE_VMBUS,  0x5, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS),
    IOCTL_VMBUS_CREATE_GPADL     = CTL_CODE(FILE_DEVICE_VMBUS,  0x6, METHOD_OUT_DIRECT, FILE_READ_ACCESS | FILE_WRITE_ACCESS),
    IOCTL_VMBUS_TEARDOWN_GPADL   = CTL_CODE(FILE_DEVICE_VMBUS,  0xA, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS),
    IOCTL_VMBUS_GET_PIPE_MODE    = CTL_CODE(FILE_DEVICE_VMBUS,  0x1, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS),
    IOCTL_VMBUS_SET_STATISTICS   = CTL_CODE(FILE_DEVICE_VMBUS,  0x17, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS),
    IOCTL_VMBUS_GET_STATISTICS   = CTL_CODE(FILE_DEVICE_VMBUS,  0x18, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS),

    //
    // Server IOCTLs
    //

    IOCTL_VMBUS_OFFER_CHANNEL      = CTL_CODE(FILE_DEVICE_VMBUS,  0x7, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS),
    IOCTL_VMBUS_QUERY_BUFFERS      = CTL_CODE(FILE_DEVICE_VMBUS,  0x8, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS),
    IOCTL_VMBUS_CREATE_PIPE        = CTL_CODE(FILE_DEVICE_VMBUS,  0xF, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS),
    IOCTL_VMBUS_GET_SAVE_INFO      = CTL_CODE(FILE_DEVICE_VMBUS, 0x11, METHOD_OUT_DIRECT, FILE_READ_ACCESS | FILE_WRITE_ACCESS),
    IOCTL_VMBUS_PUT_SAVE_INFO      = CTL_CODE(FILE_DEVICE_VMBUS, 0x12, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS),
    IOCTL_VMBUS_VDEV_SETUP         = CTL_CODE(FILE_DEVICE_VMBUS, 0x13, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS),
    IOCTL_VMBUS_GET_PIPE_SAVE_INFO = CTL_CODE(FILE_DEVICE_VMBUS, 0x14, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS),
    IOCTL_VMBUS_PUT_PIPE_SAVE_INFO = CTL_CODE(FILE_DEVICE_VMBUS, 0x15, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS),
    IOCTL_VMBUS_CONNECT_PIPE       = CTL_CODE(FILE_DEVICE_VMBUS, 0x16, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS),

    //
    // Client IOCTLs
    //

    IOCTL_VMBUS_QUERY_CHANNEL      = CTL_CODE(FILE_DEVICE_VMBUS,  0x9, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS),
    IOCTL_VMBUS_RING_BUFFER        = CTL_CODE(FILE_DEVICE_VMBUS,  0xB, METHOD_OUT_DIRECT, FILE_READ_ACCESS | FILE_WRITE_ACCESS),
    IOCTL_VMBUS_SERVER_SAVE_AREA   = CTL_CODE(FILE_DEVICE_VMBUS,  0xC, METHOD_OUT_DIRECT, FILE_READ_ACCESS | FILE_WRITE_ACCESS),
    IOCTL_VMBUS_OPEN_CHANNEL       = CTL_CODE(FILE_DEVICE_VMBUS,  0xD, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS),
    IOCTL_VMBUS_CLOSE_CHANNEL      = CTL_CODE(FILE_DEVICE_VMBUS, 0x10, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS),
};

//
// Parameters for IOCTL_VMBUS_WAIT_FOR_ACTION.  This IOCTL is sent to VMBus
// to wait for something to happen.  What that something might be is defined
// here.  When it happens, VMBus will complete the IRP.
//

typedef enum _VMBUS_CHANNEL_ACTION
{
    VmbusChannelActionNone      = 0,
    VmbusChannelActionOpened,
    VmbusChannelActionOpenFailed,
    VmbusChannelActionClosed,
    VmbusChannelActionRescinded

} VMBUS_CHANNEL_ACTION, *PVMBUS_CHANNEL_ACTION;

typedef struct
{
    VMBUS_CHANNEL_ACTION    Action;

__declspec(align(8))
    UCHAR   UserDefined[MAX_USER_DEFINED_BYTES];

} VMBUS_WAIT_FOR_ACTION_OUTPUT_PARAMETERS, *PVMBUS_WAIT_FOR_ACTION_OUTPUT_PARAMETERS;

//
// Parameters for IOCTL_VMBUS_MAP_GPADL_VIEW.  This IOCTL is sent to VMBus to
// create a mapping in the endpoint's address space for a GPADL.
//

typedef struct
{
    GPADL_HANDLE    GpadlHandle;

} VMBUS_MAP_GPADL_VIEW_INPUT_PARAMETERS, *PVMBUS_MAP_GPADL_VIEW_INPUT_PARAMETERS;

typedef struct
{
    PVOID           BaseAddress;
    UINT32          Length;

} VMBUS_MAP_GPADL_VIEW_OUTPUT_PARAMETERS, *PVMBUS_MAP_GPADL_VIEW_OUTPUT_PARAMETERS;

//
// Parameters for IOCTL_VMBUS_UNMAP_GPADL_VIEW.
//

typedef struct
{
    GPADL_HANDLE    GpadlHandle;

} VMBUS_UNMAP_GPADL_VIEW_PARAMETERS, *PVMBUS_UNMAP_GPADL_VIEW_PARAMETERS;


//
// Parameters for IOCTL_VMBUS_CREATE_GPADL
//

typedef struct
{
    //
    // This first parameter is a pointer to a GPADL_HANDLE in the sender's
    // address space.  It's declared as a UINT64 so that there can be one
    // form of this IOCTL, even for 32-bit processes running on top of a 64-bit
    // kernel.  It should be cast by the sender to PGPADL_HANDLE.
    //

    UINT64  GpadlHandleAddress;

    PVOID   PacketLibHandle;    // Optional kernel-mode only optimization

} VMBUS_CREATE_GPADL_PARAMETERS, *PVMBUS_CREATE_GPADL_PARAMETERS;

//
// Parameters for IOCTL_VMBUS_TEARDOWN_GPADL
//

typedef struct
{
    GPADL_HANDLE GpadlHandle;

} VMBUS_TEARDOWN_GPADL_PARAMETERS, *PVMBUS_TEARDOWN_GPADL_PARAMETERS;


//
// Parameters for IOCTL_VMBUS_OFFER_CHANNEL.  This IOCTL is sent to the VMBus
// root device when a server wants to offer a channel.
//

typedef struct
{
    VMBUS_CHANNEL_OFFER Offer;
    UINT64 PartitionId;

} VMBUS_OFFER_CHANNEL_INPUT_PARAMETERS, *PVMBUS_OFFER_CHANNEL_INPUT_PARAMETERS;

//
// Parameters for IOCTL_VMBUS_QUERY_BUFFERS.  This IOCTL is sent to the VMBus
// root device when a server has received notice that a client has opened the
// channel.  The server uses this IOCTL to find the GPADL tokens associated
// with the channel.
//

typedef struct
{
    GPADL_HANDLE  RingBufferGpadlHandle;
    GPADL_HANDLE  ServerSaveAreaGpadlHandle;
    UINT32        DownstreamRingBufferPageOffset;
    PLONG         InterruptFlagAddress;
    PVOID         MonitoredNotificationPages;
    LONG          InterruptFlag;
    UINT8         MonitoredNotificationId;

} VMBUS_QUERY_BUFFERS_OUTPUT_PARAMETERS, *PVMBUS_QUERY_BUFFERS_OUTPUT_PARAMETERS;

//
// Parameters for IOCTL_VMBUS_QUERY_CHANNEL.  This IOCTL is sent to VMBus by
// a client that needs to know the parameters of the channel that were
// specified by the server.
//

typedef struct
{
    VMBUS_CHANNEL_OFFER Offer;
    PLONG               InterruptFlagAddress;
    PVOID               MonitoredNotificationPages;
    LONG                InterruptFlag;
    UINT8               MonitoredNotificationId;

} VMBUS_QUERY_CHANNEL_PARAMETERS, *PVMBUS_QUERY_CHANNEL_PARAMETERS;

//
// Parameters for IOCTL_VMBUS_OPEN_CHANNEL.  This IOCTL is sent to VMBus
// by a client that wishes to open a channel.  It specifies which buffers
// the client intends to implement, which allows VMBus to know when it has
// all the other IOCTLs that the client is going to send.
//

typedef struct
{
    PVOID   PacketLibHandle;    // Optional kernel-mode only optimization
    BOOLEAN TransferPagesImplemented;
    BOOLEAN ServerSaveAreaImplemented;

__declspec(align(8))
    UCHAR   UserDefined[MAX_USER_DEFINED_BYTES];

} VMBUS_OPEN_CHANNEL_INPUT_PARAMETERS, *PVMBUS_OPEN_CHANNEL_INPUT_PARAMETERS;

//
// Parameters for IOCTL_VMBUS_TRANSFER_PAGESET.  This IOCTL is sent to VMBus
// by a client to specify a transfer page set which will be used in
// later ring buffer packets
//

typedef struct
{
    ULONG SetId;
    PVOID Buffer;
    ULONG BufferLength;

} VMBUS_TRANSFER_PAGE_SET_PARAMETERS, *PVMBUS_TRANSFER_PAGE_SET_PARAMETERS;


//
// Parameters for IOCTL_VMBUS_CREATE_PIPE.
//

typedef struct
{
    VMBUS_CHANNEL_OFFER Offer;
    UINT64              PartitionId;

} VMBUS_CREATE_PIPE_INPUT_PARAMETERS, *PVMBUS_CREATE_PIPE_INPUT_PARAMETERS;


//
// Parameters for IOCTL_VMBUS_GET_SAVE_INFO
//

typedef struct
{
    UINT64  PartitionId;
    PVOID   SaveContext;

} VMBUS_GET_SAVE_INFO_INPUT_PARAMETERS, *PVMBUS_GET_SAVE_INFO_INPUT_PARAMETERS;

typedef struct
{
    PVOID   SaveContext;
    SIZE_T  BytesToSave;
    UCHAR   SavePacket[1];

} VMBUS_GET_SAVE_INFO_OUTPUT_PARAMETERS, *PVMBUS_GET_SAVE_INFO_OUTPUT_PARAMETERS;

//
// Parameters for IOCTL_VMBUS_PUT_SAVE_INFO
//

typedef struct
{
    UINT64  PartitionId;
    SIZE_T  BytesToSave;
    UCHAR   SavePacket[1];

} VMBUS_PUT_SAVE_INFO_INPUT_PARAMETERS, *PVMBUS_PUT_SAVE_INFO_INPUT_PARAMETERS;

//
// Parameters for IOCTL_VMBUS_VDEV_SETUP
//

typedef struct
{
    UINT64  PartitionId;

} VMBUS_VDEV_SETUP_PARAMETERS, *PVMBUS_VDEV_SETUP_PARAMETERS;

//
// Parameters for IOCTL_VMBUS_RING_BUFFER
//

typedef struct
{
    UINT32  DownstreamRingBufferPageOffset;

} VMBUS_RING_BUFFER_PARAMETERS, *PVMBUS_RING_BUFFER_PARAMETERS;


//
// Parameters for IOCTL_VMBUS_GET_PIPE_SAVE_INFO
//

typedef struct
{
    SIZE_T  BytesToSave;
    UCHAR   SavePacket[1];

} VMBUS_GET_PIPE_SAVE_INFO_OUTPUT_PARAMETERS, *PVMBUS_GET_PIPE_SAVE_INFO_OUTPUT_PARAMETERS;


//
// Parameters for IOCTL_VMBUS_SET_STATISTICS
//

typedef struct
{
    BOOLEAN CollectNow;

} VMBUS_SET_STATISTICS, *PVMBUS_SET_STATISTICS;

//
// Parameters for IOCTL_GET_STATISTICS
// 

typedef struct
{
    UINT32  InterruptsReceived;
    UINT32  InterruptsSent;
    UINT32  GpadlsMapped;
    UINT32  GpadlsMappingsOutstanding;
    UINT32  ChannelOffers;
    UINT32  ChannelOffersOutstanding;

} VMBUS_GET_STATISTICS, *PVMBUS_GET_STATISTICS;

#endif

