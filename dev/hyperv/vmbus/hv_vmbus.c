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
 * Implements main vmbus entry points that is exported to the
 * open-source vmbus driver
 *
 *****************************************************************************/

/*
 * Copyright (c) 2009, Microsoft Corporation - All rights reserved.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * LICENSE-GPL in the main directory of this source tree, or the
 * BSD license (http://opensource.org/licenses/bsd-license.php).
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
	Vmbus.c

Description:
	Implements main vmbus entry points that is exported to the
	open-source vmbus driver
--*/


#include <sys/types.h>
#include <sys/systm.h>

#include <sys/smp.h>    /* for mp_ncpus extern */


#ifdef REMOVED
/* Fixme -- removed */
#include "logging.h"
#include "VersionInfo.h"
#include "VmbusPrivate.h"
#endif

/* Fixme -- may need to be updated */
#include <dev/hyperv/include/hv_osd.h>
#include <dev/hyperv/include/hv_logging.h>
/* Fixme -- currently empty */
#include "hv_version_info.h"
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


//
// Globals
//
static const char* gDriverName="vmbus";

// Windows vmbus does not defined this. We defined this to be consistent with other devices
//{c5295816-f63a-4d5f-8d1a-4daf999ca185}
static const GUID gVmbusDeviceType={
	.Data = {0x16, 0x58, 0x29, 0xc5, 0x3a, 0xf6, 0x5f, 0x4d, 0x8d, 0x1a, 0x4d, 0xaf, 0x99, 0x9c, 0xa1, 0x85}
};

//{ac3760fc-9adf-40aa-9427-a70ed6de95c5}
static const GUID gVmbusDeviceId={
	.Data = {0xfc, 0x60, 0x37, 0xac, 0xdf, 0x9a, 0xaa, 0x40, 0x94, 0x27, 0xa7, 0x0e, 0xd6, 0xde, 0x95, 0xc5}
};

static DRIVER_OBJECT* gDriver; // vmbus driver object
static DEVICE_OBJECT* gDevice; // vmbus root device


//
// Internal routines
//

static void
VmbusGetChannelInterface(
	VMBUS_CHANNEL_INTERFACE *Interface
	);

static void
VmbusGetChannelInfo(
	DEVICE_OBJECT	*DeviceObject,
	DEVICE_INFO		*DeviceInfo
	);

static void 
VmbusGetChannelOffers(
	void
	);

static int 
VmbusOnDeviceAdd(
	DEVICE_OBJECT	*Device,
	void			*AdditionalInfo
	);

static int
VmbusOnDeviceRemove(
	DEVICE_OBJECT* dev
	);

static void
VmbusOnCleanup(
	DRIVER_OBJECT* drv
	);

static int
VmbusOnISR(
	DRIVER_OBJECT* drv
	);

static void
VmbusOnMsgDPC(
	DRIVER_OBJECT* drv
	);

static void
VmbusOnEventDPC(
	DRIVER_OBJECT* drv
	);

/*++;

Name: 
	VmbusInitialize()

Description:
	Main entry point

--*/
int 
VmbusInitialize(
	DRIVER_OBJECT* drv
	)
{
	VMBUS_DRIVER_OBJECT* driver = (VMBUS_DRIVER_OBJECT*)drv; 
	int ret=0;

	DPRINT_ENTER(VMBUS);

	DPRINT_INFO(VMBUS, "+++++++ Build Date=%s %s +++++++", 
		              __DATE__, __TIME__);

	DPRINT_INFO(VMBUS, "+++++++ Build Description=%s +++++++", VersionDesc);

	DPRINT_INFO(VMBUS, "+++++++ Vmbus supported version = %d +++++++", 
			VMBUS_REVISION_NUMBER);
	DPRINT_INFO(VMBUS,  "+++++++ Vmbus using SINT %d +++++++", 
			VMBUS_MESSAGE_SINT);

	DPRINT_DBG(VMBUS, "sizeof(VMBUS_CHANNEL_PACKET_PAGE_BUFFER)=%d, sizeof(VMBUS_CHANNEL_PACKET_MULITPAGE_BUFFER)=%d",
		sizeof(VMBUS_CHANNEL_PACKET_PAGE_BUFFER), sizeof(VMBUS_CHANNEL_PACKET_MULITPAGE_BUFFER));

	drv->name = gDriverName;
	memcpy(&drv->deviceType, &gVmbusDeviceType, sizeof(GUID));

	// Setup dispatch table
	driver->Base.OnDeviceAdd		= VmbusOnDeviceAdd;
	driver->Base.OnDeviceRemove		= VmbusOnDeviceRemove;
	driver->Base.OnCleanup			= VmbusOnCleanup;
	driver->OnIsr					= VmbusOnISR;
	driver->OnMsgDpc				= VmbusOnMsgDPC;
	driver->OnEventDpc				= VmbusOnEventDPC;
	driver->GetChannelOffers		= VmbusGetChannelOffers;
	driver->GetChannelInterface		= VmbusGetChannelInterface;
	driver->GetChannelInfo			= VmbusGetChannelInfo;

	MemoryFence();	

	// Hypervisor initialization...setup hypercall page..etc
	ret = HvInit();
	if (ret != 0)
	{
		DPRINT_ERR(VMBUS, "Unable to initialize the hypervisor - 0x%x", ret);
	}

	gDriver = drv;

	DPRINT_EXIT(VMBUS);

	return ret;
}


/*++;

Name: 
	VmbusGetChannelOffers()

Description:
	Retrieve the channel offers from the parent partition

--*/

static void 
VmbusGetChannelOffers(void)
{
	DPRINT_ENTER(VMBUS);
	VmbusChannelRequestOffers();
	DPRINT_EXIT(VMBUS);
}


/*++;

Name: 
	VmbusGetChannelInterface()

Description:
	Get the channel interface

--*/
static void 
VmbusGetChannelInterface(
	VMBUS_CHANNEL_INTERFACE *Interface
	)
{
	GetChannelInterface(Interface);
}


/*++;

Name: 
	VmbusGetChannelInterface()

Description:
	Get the device info for the specified device object

--*/
static void
VmbusGetChannelInfo(
	DEVICE_OBJECT	*DeviceObject,
	DEVICE_INFO		*DeviceInfo
	)
{
	GetChannelInfo(DeviceObject, DeviceInfo);
}



/*++

Name: 
	VmbusCreateChildDevice()

Description:
	Creates the child device on the bus that represents the channel offer

--*/

DEVICE_OBJECT*
VmbusChildDeviceCreate(
	GUID DeviceType,
	GUID DeviceInstance,
	void *Context)
{
	VMBUS_DRIVER_OBJECT* vmbusDriver = (VMBUS_DRIVER_OBJECT*)gDriver; 
	
	return vmbusDriver->OnChildDeviceCreate(
		DeviceType,
		DeviceInstance,
		Context);
}


/*++

Name: 
	VmbusChildDeviceAdd()

Description:
	Registers the child device with the vmbus

--*/
int
VmbusChildDeviceAdd(
   DEVICE_OBJECT* ChildDevice)
{
	VMBUS_DRIVER_OBJECT* vmbusDriver = (VMBUS_DRIVER_OBJECT*)gDriver; 

	return vmbusDriver->OnChildDeviceAdd(gDevice, ChildDevice);
}


/*++

Name: 
	VmbusChildDeviceRemove()

Description:
	Unregisters the child device from the vmbus

--*/
void
VmbusChildDeviceRemove(
   DEVICE_OBJECT* ChildDevice)
{
	VMBUS_DRIVER_OBJECT* vmbusDriver = (VMBUS_DRIVER_OBJECT*)gDriver; 

	vmbusDriver->OnChildDeviceRemove(ChildDevice);
}

/*++

Name: 
	VmbusChildDeviceDestroy()

Description:
	Release the child device from the vmbus

--*/
//void
//VmbusChildDeviceDestroy(
//	DEVICE_OBJECT* ChildDevice
//	)
//{
//	VMBUS_DRIVER_OBJECT* vmbusDriver = (VMBUS_DRIVER_OBJECT*)gDriver; 
//	
//	vmbusDriver->OnChildDeviceDestroy(ChildDevice);
//}

/*++

Name: 
	VmbusOnDeviceAdd()

Description:
	Callback when the root bus device is added

--*/

static int
VmbusOnDeviceAdd(
	DEVICE_OBJECT	*dev,
	void			*AdditionalInfo
	)
{
	UINT32 *irqvector = (UINT32*) AdditionalInfo;
	int ret=0;
	int cpuid;

	DPRINT_ENTER(VMBUS);

	gDevice = dev;

	memcpy(&gDevice->deviceType, &gVmbusDeviceType, sizeof(GUID));
	memcpy(&gDevice->deviceInstance, &gVmbusDeviceId, sizeof(GUID));

	//strcpy(dev->name, "vmbus");
	// SynIC setup...
	// Netscaler: Calling SMP redezvous wherein memory cannot be alloc'd
	for (cpuid = 0 ; cpuid < mp_ncpus; cpuid++) {
		gHvContext.synICMessagePage[cpuid] = PageAlloc(1);
		gHvContext.synICEventPage[cpuid] = PageAlloc(1);
	}

	doOnAllCpus(HvSynicInit, (void *)(irqvector), 1, 1);

	// Connect to VMBus in the root partition
	ret = VmbusConnect();

	//VmbusSendEvent(device->localPortId+1);

	DPRINT_EXIT(VMBUS);

	return ret;
}

	
/*++

Name: 
	VmbusOnDeviceRemove()

Description:
	Callback when the root bus device is removed

--*/
int VmbusOnDeviceRemove(
	DEVICE_OBJECT* dev
	)
{
	int ret=0;

	DPRINT_ENTER(VMBUS);

	VmbusChannelReleaseUnattachedChannels();

	VmbusDisconnect();

	/*
	 * Fixme -- first parameter generates a warning
	 * first parameter of HvSynicCleanup() needs to be changed to
	 * "void *" to make warning go away.  Should do this after testing
	 * succeeds!
	 */
	doOnAllCpus(HvSynicCleanup, (void *)NULL, 1, 1);

	DPRINT_EXIT(VMBUS);

	return ret;
}


/*++

Name: 
	VmbusOnCleanup()

Description:
	Perform any cleanup when the driver is removed

--*/
void
VmbusOnCleanup(
	DRIVER_OBJECT* drv
	)
{
	//VMBUS_DRIVER_OBJECT* driver = (VMBUS_DRIVER_OBJECT*)drv;

	DPRINT_ENTER(VMBUS);

	HvCleanup();

	DPRINT_EXIT(VMBUS);
}


/*++

Name: 
	VmbusOnMsgDPC()

Description:
	DPC routine to handle messages from the hypervisior

--*/
void
VmbusOnMsgDPC(
	DRIVER_OBJECT* drv
	)
{
        //int cpu = getCpuId();
        int cpu = 0;
	void *page_addr = gHvContext.synICMessagePage[cpu];

	HV_MESSAGE* msg = (HV_MESSAGE*)page_addr + VMBUS_MESSAGE_SINT;
	HV_MESSAGE *copied;
	while (1)
	{
		if (msg->Header.MessageType == HvMessageTypeNone) // no msg
		{
			break;
		}
		else
		{
			copied = MemAllocAtomic(sizeof(HV_MESSAGE));
			if (copied == NULL)
			{
				continue;
			}

			memcpy(copied, msg, sizeof(HV_MESSAGE));
			WorkQueueQueueWorkItem(gVmbusConnection.WorkQueue, VmbusOnChannelMessage, (void*)copied);
		}
				
		msg->Header.MessageType = HvMessageTypeNone;

		// Make sure the write to MessageType (ie set to HvMessageTypeNone) happens
		// before we read the MessagePending and EOMing. Otherwise, the EOMing will not deliver
		// any more messages since there is no empty slot
		MemoryFence();

		if (msg->Header.MessageFlags.MessagePending)
		{
			// This will cause message queue rescan to possibly deliver another msg from the hypervisor
			WriteMsr(HV_X64_MSR_EOM, 0);
		}
	}
}

/*++

Name: 
	VmbusOnEventDPC()

Description:
	DPC routine to handle events from the hypervisior

--*/
void
VmbusOnEventDPC(
	DRIVER_OBJECT* drv
	)
{
	// TODO: Process any events
	VmbusOnEvents();
}


/*++

Name: 
	VmbusOnISR()

Description:
	ISR routine

--*/
int CheckEvents(void);

int
VmbusOnISR(
	DRIVER_OBJECT* drv
	)
{
	//VMBUS_DRIVER_OBJECT* driver = (VMBUS_DRIVER_OBJECT*)drv;

	int ret=0;
	int cpu = getCpuId();
	//struct page* page;
	void *page_addr;
	HV_MESSAGE* msg;
	HV_SYNIC_EVENT_FLAGS* event;

	//page = SynICMessagePage[0];
	//page_addr = page_address(page);
	page_addr = gHvContext.synICMessagePage[cpu];
	msg = (HV_MESSAGE*)page_addr + VMBUS_MESSAGE_SINT;

	DPRINT_ENTER(VMBUS);

	// Check if there are actual msgs to be process
	if (msg->Header.MessageType != HvMessageTypeNone)
	{
		DPRINT_DBG(VMBUS, "received msg type %d size %d", msg->Header.MessageType, msg->Header.PayloadSize);
		ret |= 0x1;
	}

	// TODO: Check if there are events to be process
	page_addr = gHvContext.synICEventPage[cpu];
	event = (HV_SYNIC_EVENT_FLAGS*)page_addr + VMBUS_MESSAGE_SINT;

	// Since we are a child, we only need to check bit 0
	//printf("VMBUS OnISR: event flags: %x\n", event->Flags32[0]);
	if (BitTestAndClear(&event->Flags32[0], 0))
	{
		DPRINT_DBG(VMBUS, "received event %d", event->Flags32[0]);
		/* NetScaler */
		if (CheckEvents())
		ret |= 0x2;
	}

	DPRINT_EXIT(VMBUS);
	return ret;
}

// eof

