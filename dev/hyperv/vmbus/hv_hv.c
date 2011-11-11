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
 * Implements low-level interactions with windows hypervisor
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

/*++

File:
	Hv.c

Description:
	Implements low-level interactions with windows hypervisor

--*/

/* Fixme:  Added these includes to get memset, MAXCPU */
#include <sys/param.h>
//#include <sys/systm.h>
//#include <sys/sockio.h>
#include <sys/mbuf.h>
//#include <sys/malloc.h>
//#include <sys/module.h>
//#include <sys/kernel.h>
//#include <sys/socket.h>
//#include <sys/queue.h>
//#include <sys/lock.h>
//#include <sys/sx.h>

//#include <net/if.h>
//#include <net/if_arp.h>


#ifdef REMOVED
#include "logging.h"
#include "VmbusPrivate.h"
#include "../../../src/config.h"
#include <sys/time.h>
#include <sys/pcpu.h>
#endif

#include <dev/hyperv/include/hv_osd.h>
#include <dev/hyperv/include/hv_logging.h>
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

// Fixme -- causes problems
//#include <sys/param.h>

// Fixme -- this may need some work
// Fixme -- this may not be needed after all
#include <dev/hyperv/include/hv_config.h>

#include <sys/time.h>
#include <sys/pcpu.h>

//
// Globals
//

// The one and only
HV_CONTEXT gHvContext={
	.SynICInitialized = FALSE,
	.HypercallPage = NULL,
	.SignalEventParam = NULL,
	.SignalEventBuffer = NULL,
};


/*++

Name: 
	HvQueryHypervisorPresence()

Description:
	Query the cpuid for presense of windows hypervisor

--*/
int
HvQueryHypervisorPresence (
    void
    )
{
    unsigned int eax;
    unsigned int ebx;
    unsigned int ecx;
    unsigned int edx;
    unsigned int op;

    eax = 0;
    ebx = 0;
    ecx = 0;
    edx = 0;
    op = HvCpuIdFunctionVersionAndFeatures;
    do_cpuid(op, &eax, &ebx, &ecx, &edx);

	return (ecx & HV_PRESENT_BIT);
}


/*++

Name: 
	HvQueryHypervisorInfo()

Description:
	Get version info of the windows hypervisor 

--*/
static int
HvQueryHypervisorInfo (
    void
    )
{
    unsigned int eax;
    unsigned int ebx;
    unsigned int ecx;
    unsigned int edx;
    unsigned int maxLeaf;
    unsigned int op;

    //
    // Its assumed that this is called after confirming that Viridian is present.
    // Query id and revision.
    //

    eax = 0;
    ebx = 0;
    ecx = 0;
    edx = 0;
    op = HvCpuIdFunctionHvVendorAndMaxFunction;
    do_cpuid(op, &eax, &ebx, &ecx, &edx);

    DPRINT_INFO(VMBUS, "Vendor ID: %c%c%c%c%c%c%c%c%c%c%c%c",
           (ebx & 0xFF),
           ((ebx >> 8) & 0xFF),
           ((ebx >> 16) & 0xFF),
           ((ebx >> 24) & 0xFF),
           (ecx & 0xFF),
           ((ecx >> 8) & 0xFF),
           ((ecx >> 16) & 0xFF),
           ((ecx >> 24) & 0xFF),
           (edx & 0xFF),
           ((edx >> 8) & 0xFF),
           ((edx >> 16) & 0xFF),
           ((edx >> 24) & 0xFF));

    maxLeaf = eax;
    eax = 0;
    ebx = 0;
    ecx = 0;
    edx = 0;
    op = HvCpuIdFunctionHvInterface;
    do_cpuid(op, &eax, &ebx, &ecx, &edx);

    DPRINT_INFO(VMBUS, "Interface ID: %c%c%c%c",
           (eax & 0xFF),
           ((eax >> 8) & 0xFF),
           ((eax >> 16) & 0xFF),
           ((eax >> 24) & 0xFF));

	 if (maxLeaf >= HvCpuIdFunctionMsHvVersion) {
        eax = 0;
        ebx = 0;
        ecx = 0;
        edx = 0;
        op = HvCpuIdFunctionMsHvVersion;
        do_cpuid(op, &eax, &ebx, &ecx, &edx);
        DPRINT_INFO(VMBUS, "OS Build:%d-%d.%d-%d-%d.%d",
               eax,
               ebx >> 16,
               ebx & 0xFFFF,
               ecx,
               edx >> 24,
               edx & 0xFFFFFF);
    }
    return maxLeaf;
}

/*++

Name: 
	HvDoHypercall()

Description:
	Invoke the specified hypercall 

--*/
static UINT64
HvDoHypercall (
    UINT64  Control,
    void*   Input,
    void*   Output
    )
{
#ifdef __x86_64__
    UINT64 hvStatus=0;
    UINT64 inputAddress = (Input)? GetPhysicalAddress(Input) : 0;
	UINT64 outputAddress = (Output)? GetPhysicalAddress(Output) : 0;
    volatile void* hypercallPage = gHvContext.HypercallPage;

    DPRINT_DBG(VMBUS, "Hypercall <control %llx input phys %llx virt %p output phys %llx virt %p hypercall %p>", 
		Control,
		inputAddress,
		Input,
		outputAddress,
		Output,
		hypercallPage);

	__asm__ __volatile__ ("mov %0, %%r8" : : "r" (outputAddress):  "r8");
	__asm__ __volatile__ ("call *%3" : "=a"(hvStatus): "c" (Control), "d" (inputAddress), "m" (hypercallPage));

    DPRINT_DBG(VMBUS, "Hypercall <return %llx>",  hvStatus);

    return hvStatus;

#else

    UINT32 controlHi = Control >> 32;
    UINT32 controlLo = Control & 0xFFFFFFFF;
    UINT32 hvStatusHi = 1;
    UINT32 hvStatusLo = 1;
    UINT64 inputAddress = (Input) ? GetPhysicalAddress(Input) : 0;
    UINT32 inputAddressHi = inputAddress >> 32;
    UINT32 inputAddressLo = inputAddress & 0xFFFFFFFF;
	UINT64 outputAddress = (Output) ?GetPhysicalAddress(Output) : 0;
    UINT32 outputAddressHi = outputAddress >> 32;
    UINT32 outputAddressLo = outputAddress & 0xFFFFFFFF;
    volatile void* hypercallPage = gHvContext.HypercallPage;

    DPRINT_DBG(VMBUS, "Hypercall <control %llx input %p output %p>", 
		Control,
		Input,
		Output);

	__asm__ __volatile__ ("call *%8" : "=d"(hvStatusHi), "=a"(hvStatusLo) : "d" (controlHi), "a" (controlLo), "b" (inputAddressHi), "c" (inputAddressLo), "D"(outputAddressHi), "S"(outputAddressLo), "m" (hypercallPage));

	
    DPRINT_DBG(VMBUS, "Hypercall <return %llx>",  hvStatusLo | ((UINT64)hvStatusHi << 32));

    return (hvStatusLo | ((UINT64)hvStatusHi << 32));
#endif // __x86_64__
}

/*++

Name: 
	HvInit()

Description:
	Main initialization routine. This routine must be called
	before any other routines in here are called

--*/

int
HvInit (
    void
    )
{
	int ret=0;
    int maxLeaf;
	HV_X64_MSR_HYPERCALL_CONTENTS hypercallMsr;
	void* virtAddr=0;

	DPRINT_ENTER(VMBUS);

	memset(gHvContext.synICEventPage, 0, sizeof(HANDLE)*MAX_NUM_CPUS);
	memset(gHvContext.synICMessagePage, 0, sizeof(HANDLE)*MAX_NUM_CPUS);

	if (!HvQueryHypervisorPresence())
	{
		DPRINT_ERR(VMBUS, "No Windows hypervisor detected!!");
		goto Cleanup;
	}
		
	DPRINT_INFO(VMBUS, "Windows hypervisor detected! Retrieving more info...");

	maxLeaf = HvQueryHypervisorInfo();
	//HvQueryHypervisorFeatures(maxLeaf);

	// Determine if we are running on xenlinux (ie x2v shim) or native linux
	gHvContext.GuestId = ReadMsr(HV_X64_MSR_GUEST_OS_ID);  

	if (gHvContext.GuestId == 0)
	{
		DPRINT_INFO(VMBUS, "Setting Guest OS Id to HV_LINUX_GUEST_ID");
		// Write our OS info
		WriteMsr(HV_X64_MSR_GUEST_OS_ID, HV_LINUX_GUEST_ID);  

		gHvContext.GuestId = HV_LINUX_GUEST_ID;
	}

	// See if the hypercall page is already set
	hypercallMsr.AsUINT64 = ReadMsr(HV_X64_MSR_HYPERCALL);

	if (gHvContext.GuestId == HV_LINUX_GUEST_ID)
	{
		DPRINT_INFO(VMBUS, "Guest OS Id is HV_LINUX_GUEST_ID");
		// Allocate the hypercall page memory
		//virtAddr = PageAlloc(1);
		virtAddr = VirtualAllocExec(PAGE_SIZE);
		
		if (!virtAddr)
		{
			DPRINT_ERR(VMBUS, "unable to allocate hypercall page!!");
			goto Cleanup;
		}

		hypercallMsr.Enable = 1;
		//hypercallMsr.GuestPhysicalAddress = Logical2PhysicalAddr(virtAddr) >> PAGE_SHIFT;
		hypercallMsr.GuestPhysicalAddress = Virtual2Physical(virtAddr) >> PAGE_SHIFT;
		WriteMsr(HV_X64_MSR_HYPERCALL, hypercallMsr.AsUINT64);

		// Confirm that hypercall page did get set up.
		hypercallMsr.AsUINT64 = 0;
		hypercallMsr.AsUINT64 = ReadMsr(HV_X64_MSR_HYPERCALL);

		if (!hypercallMsr.Enable)
		{
			DPRINT_ERR(VMBUS, "unable to set hypercall page!!");
			goto Cleanup;
		}

		gHvContext.HypercallPage = virtAddr;
	}
	else
	{
		DPRINT_ERR(VMBUS, "Unknown guest id (0x%llx)!!", gHvContext.GuestId);
		goto Cleanup;
	}

	DPRINT_INFO(VMBUS, "Hypercall page VA=0x%08x, PA=0x%08x",
               (unsigned long)gHvContext.HypercallPage,
               (unsigned long)hypercallMsr.GuestPhysicalAddress << PAGE_SHIFT);

	// Setup the global signal event param for the signal event hypercall
	gHvContext.SignalEventBuffer = MemAlloc(sizeof(HV_INPUT_SIGNAL_EVENT_BUFFER));
	if (!gHvContext.SignalEventBuffer)
	{
		goto Cleanup;
	}

	gHvContext.SignalEventParam = (PHV_INPUT_SIGNAL_EVENT)(ALIGN_UP((ULONG_PTR)gHvContext.SignalEventBuffer, HV_HYPERCALL_PARAM_ALIGN));
	gHvContext.SignalEventParam->ConnectionId.AsUINT32 = 0;
	gHvContext.SignalEventParam->ConnectionId.u.Id = VMBUS_EVENT_CONNECTION_ID;
	gHvContext.SignalEventParam->FlagNumber = 0;
	gHvContext.SignalEventParam->RsvdZ = 0;

	//DPRINT_DBG(VMBUS, "My id %llu", HvGetCurrentPartitionId());

	DPRINT_EXIT(VMBUS);

	return ret;

Cleanup:
	if (virtAddr)
	{
		if (hypercallMsr.Enable)
		{
			hypercallMsr.AsUINT64 = 0;
			WriteMsr(HV_X64_MSR_HYPERCALL, hypercallMsr.AsUINT64);
		}

		VirtualFree(virtAddr);
	}
	ret = -1;
	DPRINT_EXIT(VMBUS);

	return ret;
}


/*++

Name: 
	HvCleanup()

Description:
	Cleanup routine. This routine is called normally during driver unloading or exiting.

--*/
void
HvCleanup (
    void
    )
{
	HV_X64_MSR_HYPERCALL_CONTENTS hypercallMsr;

	DPRINT_ENTER(VMBUS);

	if (gHvContext.SignalEventBuffer)
	{
		MemFree(gHvContext.SignalEventBuffer);
		gHvContext.SignalEventBuffer = NULL;
		gHvContext.SignalEventParam = NULL;
	}

	if (gHvContext.GuestId == HV_LINUX_GUEST_ID)
	{
		if (gHvContext.HypercallPage) 
		{
			hypercallMsr.AsUINT64 = 0;
			WriteMsr(HV_X64_MSR_HYPERCALL, hypercallMsr.AsUINT64);
			VirtualFree(gHvContext.HypercallPage);
			gHvContext.HypercallPage = NULL;
		}
	}

	DPRINT_EXIT(VMBUS);

}


/*++

Name: 
	HvPostMessage()

Description:
	Post a message using the hypervisor message IPC. This
	involves a hypercall.

--*/
HV_STATUS
HvPostMessage(
	HV_CONNECTION_ID connectionId,
	HV_MESSAGE_TYPE  messageType,
	PVOID            payload,
	SIZE_T           payloadSize
	)
{
	struct alignedInput {
		UINT64					alignment8;
		HV_INPUT_POST_MESSAGE	msg;
	};

	PHV_INPUT_POST_MESSAGE alignedMsg;
	HV_STATUS status;
	ULONG_PTR addr;

	if (payloadSize > HV_MESSAGE_PAYLOAD_BYTE_COUNT)
	{
		return -1;
	}

	addr = (ULONG_PTR)MemAllocAtomic(sizeof(struct alignedInput));

	if (!addr)
	{
		return -1;
	}

	alignedMsg = (PHV_INPUT_POST_MESSAGE)(ALIGN_UP(addr, HV_HYPERCALL_PARAM_ALIGN));

	alignedMsg->ConnectionId = connectionId;
	alignedMsg->MessageType = messageType;
	alignedMsg->PayloadSize = payloadSize;
	memcpy((void*)alignedMsg->Payload, payload, payloadSize);

//	if (((unsigned int)alignedMsg & ~0x0fff) != ((unsigned int)((char *)alignedMsg+sizeof(HV_INPUT_POST_MESSAGE)-1) & ~0xfff)) {
//		printf("alignedMsg: %p, %p\n", alignedMsg, &alignedMsg[1]);
//	}
	status = HvDoHypercall(HvCallPostMessage, alignedMsg, 0) & 0xFFFF;

	MemFree((void*)addr);

	return status;
}


/*++

Name: 
	HvSignalEvent()

Description:
	Signal an event on the specified connection using the hypervisor event IPC. This
	involves a hypercall.

--*/
HV_STATUS
HvSignalEvent(
	)
{
	HV_STATUS status;

	status = HvDoHypercall(HvCallSignalEvent, gHvContext.SignalEventParam, 0) & 0xFFFF;

	return status;
}


/*++

Name: 
	HvSynicInit()

Description:
	Initialize the Synthethic Interrupt Controller. If it is already initialized by
	another entity (ie x2v shim), we need to retrieve the initialized message and event pages.
	Otherwise, we create and initialize the message and event pages.

--*/
/* Fixme:  Added for NetScaler, then FreeBSD port */
#ifdef DPRINT_DBG
#undef DPRINT_DBG
#define DPRINT_DBG(...)
#endif
#ifdef DPRINT_ENTER
#undef DPRINT_ENTER
#define DPRINT_ENTER(mod)
#endif
#ifdef DPRINT_EXIT
#undef DPRINT_EXIT
#define DPRINT_EXIT(mod)
#endif

void
HvSynicInit (
	void *irqArg
	)
{    
	UINT64		version;
	HV_SYNIC_SIMP	simp;
	HV_SYNIC_SIEFP	siefp;
	HV_SYNIC_SINT	sharedSint;
#ifdef REMOVED
/* Fixme:  Removed to mitigate warning */
	HV_SYNIC_SINT	sharedSint1;
#endif
	HV_SYNIC_SCONTROL sctrl;
#ifdef REMOVED
/* Fixme:  Removed to mitigate warning */
	UINT64		guestID;
#endif
	UINT32 irqVector = *((UINT32 *)(irqArg));
	int cpu = PCPU_GET(cpuid);

	DPRINT_ENTER(VMBUS);

	if (!gHvContext.HypercallPage)
	{
		DPRINT_EXIT(VMBUS);
		return;
	}

	// Check the version
	version = ReadMsr(HV_X64_MSR_SVERSION);

//	DPRINT_INFO(VMBUS, "SynIC version: %llx", version);

	{
//		DPRINT_INFO(VMBUS, "set up SIMP and SIEFP.");

		/*
		 * Fixme:  lis21 code allocates the following here.  In
		 * our code, the caller of this function allocates these
		 * before the call.
		 */

#ifdef REMOVED
		gHvContext.synICMessagePage[cpu] = PageAllocAtomic();
		if (gHvContext.synICMessagePage[cpu] == NULL)
		{
			DPRINT_ERR(VMBUS, "unable to allocate SYNIC message page!!");
			goto Cleanup;
		}

		gHvContext.synICEventPage[cpu] = PageAllocAtomic();
		if (gHvContext.synICEventPage[cpu] == NULL)
		{
			DPRINT_ERR(VMBUS, "unable to allocate SYNIC event page!!");
			goto Cleanup;
		}
#endif

		//
		// Setup the Synic's message page
		//
		simp.AsUINT64 = ReadMsr(HV_X64_MSR_SIMP);
		simp.SimpEnabled = 1;
		simp.BaseSimpGpa =
		    GetPhysicalAddress(gHvContext.synICMessagePage[cpu]) >>
		    PAGE_SHIFT;

		DPRINT_DBG(VMBUS, "HV_X64_MSR_SIMP msr set to: %llx",
		    simp.AsUINT64);

		WriteMsr(HV_X64_MSR_SIMP, simp.AsUINT64);

		//
		// Setup the Synic's event page
		//
		siefp.AsUINT64 = ReadMsr(HV_X64_MSR_SIEFP);
		siefp.SiefpEnabled = 1;
		siefp.BaseSiefpGpa =
		    GetPhysicalAddress(gHvContext.synICEventPage[cpu]) >>
		    PAGE_SHIFT;

		DPRINT_DBG(VMBUS, "HV_X64_MSR_SIEFP msr set to: %llx",
		    siefp.AsUINT64);

		WriteMsr(HV_X64_MSR_SIEFP, siefp.AsUINT64);
	}
	//
	// Set up the interception SINT.
	//
	//WriteMsr((HV_X64_MSR_SINT0 + HV_SYNIC_INTERCEPTION_SINT_INDEX),
	//             interceptionSint.AsUINT64);

	//
	// Set up the shared SINT.
	// 
//	DPRINT_INFO(VMBUS, "setup shared SINT.");
	sharedSint.AsUINT64 = ReadMsr(HV_X64_MSR_SINT0 + VMBUS_MESSAGE_SINT);

	sharedSint.AsUINT64 = 0;
	sharedSint.Vector = irqVector; //HV_SHARED_SINT_IDT_VECTOR + 0x20;
	sharedSint.Masked = FALSE;
	sharedSint.AutoEoi = TRUE;

	DPRINT_DBG(VMBUS, "HV_X64_MSR_SINT1 msr set to: %llx", sharedSint.AsUINT64);

	WriteMsr(HV_X64_MSR_SINT0 + VMBUS_MESSAGE_SINT, sharedSint.AsUINT64);

	// Enable the global synic bit
	sctrl.AsUINT64 = ReadMsr(HV_X64_MSR_SCONTROL);
	sctrl.Enable = 1;

	WriteMsr(HV_X64_MSR_SCONTROL, sctrl.AsUINT64);

	gHvContext.SynICInitialized = TRUE;

//	sharedSint1.AsUINT64 = ReadMsr(HV_X64_MSR_SINT0 + VMBUS_MESSAGE_SINT);
//	printf("HV: Vec: %x, Masked: %x, EOI: %x\n",
//	    sharedSint1.Vector, sharedSint1.Masked, sharedSint1.AutoEoi);

	DPRINT_EXIT(VMBUS);

	return;

#ifdef REMOVED
/* Fixme:  Removed to mitigate warning */
Cleanup:
#endif
	DPRINT_EXIT(VMBUS);

	return;
}

/*++

Name: 
	HvSynicCleanup()

Description:
	Cleanup routine for HvSynicInit().

--*/
VOID
HvSynicCleanup(
	VOID *arg
	)
{
	HV_SYNIC_SINT	sharedSint;
	HV_SYNIC_SIMP	simp;
	HV_SYNIC_SIEFP	siefp;
	int cpu = PCPU_GET(cpuid);

	DPRINT_ENTER(VMBUS);

	if (!gHvContext.SynICInitialized)
	{
		DPRINT_EXIT(VMBUS);
		return;
	}

	sharedSint.AsUINT64 = ReadMsr(HV_X64_MSR_SINT0 + VMBUS_MESSAGE_SINT);

	sharedSint.Masked = 1;

//KYS: Need to correctly cleanup in the case of SMP!!!
	// Disable the interrupt
	WriteMsr(HV_X64_MSR_SINT0 + VMBUS_MESSAGE_SINT, sharedSint.AsUINT64);

	// Disable and free the resources only if we are running as native linux
	// since in xenlinux, we are sharing the resources with the x2v shim
	if (gHvContext.GuestId == HV_LINUX_GUEST_ID)
	{
		simp.AsUINT64 = ReadMsr(HV_X64_MSR_SIMP);
		simp.SimpEnabled = 0;
		simp.BaseSimpGpa = 0;

		WriteMsr(HV_X64_MSR_SIMP, simp.AsUINT64);

		siefp.AsUINT64 = ReadMsr(HV_X64_MSR_SIEFP);
		siefp.SiefpEnabled = 0;
		siefp.BaseSiefpGpa = 0;

		WriteMsr(HV_X64_MSR_SIEFP, siefp.AsUINT64);

		PageFree(gHvContext.synICMessagePage[cpu], 1);
		PageFree(gHvContext.synICEventPage[cpu], 1);
	}

	DPRINT_EXIT(VMBUS);
}


// eof
