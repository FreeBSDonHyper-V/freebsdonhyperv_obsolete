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
 * HyperV vmbus header file
 *
 *****************************************************************************/

#ifndef __HV_VMBUS_H__
#define __HV_VMBUS_H__

#ifdef REMOVED
// Fixme -- removed
#include <sys/bus.h>
#include"vmbusvar.h"
#include"VmbusApi.h"
#endif

struct driver_context { //blkvsc_drv.c 
	GUID	class_id;
	UINT32	driver;
	int		(*probe)(struct device*);
	int		(*remove)(struct device* );
	void	(*shutdown)(struct device* );
};	

struct device_context {
        GUID                     class_id;
        GUID                     device_id;
        int                      probe_error;
        struct task              probe_failed_work_item;
        device_t                 device;
        DEVICE_OBJECT            device_obj;
};


/*
 * This function defined in vmbus_drv_freebsd.c and used in
 * Netvsc_drv_freebsd.c, Blkvsc_drv.c. 
 */
void vmbus_get_interface(VMBUS_CHANNEL_INTERFACE *);

#define container_of(ptr, type, member) ({                      \
        typeof( ((type *)0)->member ) *__mptr = (ptr);  \
        (type *)( (char *)__mptr - offsetof(type,member) );})

static inline struct device_context *to_device_context(DEVICE_OBJECT *device_obj
)
{
        return container_of(device_obj, struct device_context, device_obj);
}

static inline struct device_context *device_to_device_context(device_t *device)
{
        return container_of(device, struct device_context, device);
}



enum {
		VMBUS_IVAR_TYPE,
		VMBUS_IVAR_INSTANCE,
		VMBUS_IVAR_NODE,
		VMBUS_IVAR_DEVCTX
};

#ifdef REMOVED
/*
 * Fixme:  Defined as having different values elsewhere.  If these need to
 * have unique values, they must be renamed.
 */
/* Fixme:  Defined as a complicated bit mask in hv_logging.c */
#define ALL_MODULES 5
/* Fixme:  Defined as 6 in hv_logging.c */
#define INFO_LVL  1
#endif

#define VMBUS_ACCESSOR(var, ivar, type) \
		__BUS_ACCESSOR(vmbus, var, VMBUS, ivar, type)

VMBUS_ACCESSOR(type, TYPE,  const char *)
VMBUS_ACCESSOR(devctx, DEVCTX,  struct device_context *)

void vmbus_child_driver_register(struct driver_context* driver_ctx);

#endif  /* __HV_VMBUS_H__ */

