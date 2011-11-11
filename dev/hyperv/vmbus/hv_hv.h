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
 * HyperV low-level hypervisor interface definition file 
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

#ifndef __HV_HV_H__
#define __HV_HV_H__

#ifdef REMOVED
/* Fixme -- removed */
#include "osd.h"

#include "HvTypes.h"
#include "HvStatus.h"
//#include "HvVmApi.h"
//#include "HvKeApi.h"
//#include "HvMmApi.h"
//#include "HvCpuApi.h"
#include "HvHalApi.h"
#include "HvVpApi.h"
//#include "HvTrApi.h"
#include "HvSynicApi.h"
//#include "HvAmApi.h" 
//#include "HvHkApi.h"
//#include "HvValApi.h"
#include "HvHcApi.h"
#include "HvPtApi.h"
#endif

/*
 * Fixme -- not at all thrilled about including these in header file
 */
#include <dev/hyperv/include/hv_types.h>
#include <dev/hyperv/include/hv_status.h>

#include <dev/hyperv/include/hv_hal_api.h>
#include <dev/hyperv/include/hv_vp_api.h>

#include <dev/hyperv/include/hv_synic_api.h>

#include <dev/hyperv/include/hv_hc_api.h>
/* Fixme -- not sure this one is needed */
#include <dev/hyperv/include/hv_pt_api.h>


enum
{
    VMBUS_MESSAGE_CONNECTION_ID = 1,
    VMBUS_MESSAGE_PORT_ID       = 1,
    VMBUS_EVENT_CONNECTION_ID   = 2,
    VMBUS_EVENT_PORT_ID         = 2,
    VMBUS_MONITOR_CONNECTION_ID = 3,
    VMBUS_MONITOR_PORT_ID       = 3,
    VMBUS_MESSAGE_SINT          = 2
};
// 
// #defines
//
#define HV_PRESENT_BIT				0x80000000

#define HV_XENLINUX_GUEST_ID_LO     0x00000000
#define HV_XENLINUX_GUEST_ID_HI		0x0B00B135
#define HV_XENLINUX_GUEST_ID		(((UINT64)HV_XENLINUX_GUEST_ID_HI << 32) | HV_XENLINUX_GUEST_ID_LO) 

#define HV_LINUX_GUEST_ID_LO		0x00000000
#define HV_LINUX_GUEST_ID_HI		0xB16B00B5
#define HV_LINUX_GUEST_ID			(((UINT64)HV_LINUX_GUEST_ID_HI << 32) | HV_LINUX_GUEST_ID_LO) 

#define HV_CPU_POWER_MANAGEMENT     (1 << 0)
#define HV_RECOMMENDATIONS_MAX      4

#define HV_X64_MAX                  5
#define HV_CAPS_MAX                 8


#define HV_HYPERCALL_PARAM_ALIGN	sizeof(UINT64)

//
// Service definitions
//
#define HV_SERVICE_PARENT_PORT (0)
#define HV_SERVICE_PARENT_CONNECTION (0)

#define HV_SERVICE_CONNECT_RESPONSE_SUCCESS             (0)
#define HV_SERVICE_CONNECT_RESPONSE_INVALID_PARAMETER   (1)
#define HV_SERVICE_CONNECT_RESPONSE_UNKNOWN_SERVICE     (2)
#define HV_SERVICE_CONNECT_RESPONSE_CONNECTION_REJECTED (3)

#define HV_SERVICE_CONNECT_REQUEST_MESSAGE_ID		(1)
#define HV_SERVICE_CONNECT_RESPONSE_MESSAGE_ID		(2)
#define HV_SERVICE_DISCONNECT_REQUEST_MESSAGE_ID	(3)
#define HV_SERVICE_DISCONNECT_RESPONSE_MESSAGE_ID	(4)
#define HV_SERVICE_MAX_MESSAGE_ID					(4)

#define HV_SERVICE_PROTOCOL_VERSION (0x0010)
#define HV_CONNECT_PAYLOAD_BYTE_COUNT 64

//#define VMBUS_REVISION_NUMBER	6
//#define VMBUS_PORT_ID			11		// Our local vmbus's port and connection id. Anything >0 is fine

// 628180B8-308D-4c5e-B7DB-1BEB62E62EF4
static const GUID VMBUS_SERVICE_ID = {.Data = {0xb8, 0x80, 0x81, 0x62, 0x8d, 0x30, 0x5e, 0x4c, 0xb7, 0xdb, 0x1b, 0xeb, 0x62, 0xe6, 0x2e, 0xf4} };

#define MAX_NUM_CPUS	 4


typedef struct {
	UINT64					Align8;
	HV_INPUT_SIGNAL_EVENT	Event;
} HV_INPUT_SIGNAL_EVENT_BUFFER;

typedef struct {
	UINT64	GuestId;			// XenLinux or native Linux. If XenLinux, the hypercall and synic pages has already been initialized
	void*	HypercallPage;

	BOOL	SynICInitialized;
	// This is used as an input param to HvCallSignalEvent hypercall. The input param is immutable 
	// in our usage and must be dynamic mem (vs stack or global). 
	HV_INPUT_SIGNAL_EVENT_BUFFER *SignalEventBuffer;
	HV_INPUT_SIGNAL_EVENT *SignalEventParam; // 8-bytes aligned of the buffer above

	HANDLE	synICMessagePage[MAX_NUM_CPUS];
	HANDLE	synICEventPage[MAX_NUM_CPUS];
} HV_CONTEXT;


//
// Inline routines
//
static inline unsigned long long ReadMsr(int msr)
{
	unsigned long long val;
	
	RDMSR(msr, val);

	return val;
}

static inline void WriteMsr(int msr, UINT64 val)
{
	WRMSR(msr, val);

	return;
}

//
// Hv Interface
//
INTERNAL int
HvInit(
    VOID
    );

INTERNAL VOID
HvCleanup(
    VOID
    );

INTERNAL HV_STATUS
HvPostMessage(
	HV_CONNECTION_ID connectionId,
	HV_MESSAGE_TYPE  messageType,
	PVOID            payload,
	SIZE_T           payloadSize
	);

INTERNAL HV_STATUS
HvSignalEvent(
	VOID
	);

INTERNAL void 
HvSynicInit(
	void * irqArg
	);

INTERNAL VOID
HvSynicCleanup(
	VOID *arg
	);

/*
 * Externs
 */
extern HV_CONTEXT gHvContext;

extern int  HvQueryHypervisorPresence(void);


#endif  /* __HV_HV_H__ */

