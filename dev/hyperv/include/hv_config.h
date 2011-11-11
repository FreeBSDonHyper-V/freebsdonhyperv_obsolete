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
 * HyperV X2V shim code
 * Fixme:  Likely not needed, at least for vmbus and channel
 *
 *****************************************************************************/

/*++

Copyright 2008 Microsoft Corporation. All Rights Reserved.

--*/

#ifndef __HV_CONFIG_H__
#define __HV_CONFIG_H__

//
// X2V shim is Xen 3.03 compatible.
//

#define X2V_XEN_VERSION 305

//
// Maximum number of event ports supported by X2V shim.
//

#define MAX_EVENT_CHANNEL_PORTS 32

//
// Start of X2V virtual address space. 
// XenLinux is not allowed access to this region. 
// Size of X2V VA is X2V_PHYSICAL_SIZE.
//

#if defined(X86_PAE)

#define X2V_VIRTUAL_START 0xF5800000

#elif defined(X86_64)

#define X2V_VIRTUAL_START 0xFFFF830000000000

#else

#define X2V_VIRTUAL_START 0xFC000000

#endif

#if defined(X86_64)

#define X2V_VIRTUAL_END   0xFFFF87FFFFFFFFFF

#else

#define X2V_VIRTUAL_END   0xFFFFFFFF

#endif

//
// All fixed mappings are done starting at this VA. Fixed map size is 4MB.
//

#define X2V_FIXED_MAP_BASE      X2V_VIRTUAL_START
#define X2V_FIXED_MAP_SIZE      X2V_PAGE_SIZE
#define X2V_FIXED_MAP_COUNT     (X2V_FIXED_MAP_SIZE / sizeof(PHYSICAL_ADDRESS))

//
// All dynamic mappings are done starting at this VA.
//

#define X2V_DYNAMIC_MAP_BASE    (X2V_FIXED_MAP_BASE + _4MB)

//
// Size of dynamic map page table in bytes.
//

#if defined(X86_32)

#define X2V_DYNAMIC_MAP_SIZE    (4096 * 8)

#elif defined(X86_64)

#define X2V_DYNAMIC_MAP_SIZE    (4096 * 128)

#else

#define X2V_DYNAMIC_MAP_SIZE    (4096 * 64)

#endif // defined(X86_PAE)

//
// Number of entries in the dynamic map page table.
//

#define X2V_DYNAMIC_MAP_COUNT  (X2V_DYNAMIC_MAP_SIZE / sizeof(PHYSICAL_ADDRESS))

//
// Maximum number of IO APICs supported.
//

#define X2V_IOAPIC_COUNT    256  

//
// X2V shim gets compiled to be loaded at this virtual address.
//

#if defined(X86_64)

//
// This needs to match the number in x2v-64.lds
//

#define X2V_VIRTUAL_BASE  	(X2V_VIRTUAL_START + 8 * X2V_PHYSICAL_BASE)

#else

#define X2V_VIRTUAL_BASE    0xFF000000

#endif

//
// X2V shim gets loaded by GRUB at this physical address.
//

#define X2V_PHYSICAL_BASE	0x04000000

//
// Amount of physical memory reserved for X2V.
//

#define X2V_PHYSICAL_SIZE   0x01000000

#define X2V_PHYSICAL_END    (X2V_PHYSICAL_BASE + X2V_PHYSICAL_SIZE)

//
// This represents offset thats used to adjust addresses before paging is
// enabled.
//

#define X2V_VIRTUAL_BIAS	(X2V_VIRTUAL_BASE - X2V_PHYSICAL_BASE)

//
// Used to compute size of a memory page.
//

#define X2V_PAGE_SHIFT		12

//
// Size of memory page.
//

#define X2V_PAGE_SIZE		(1 << X2V_PAGE_SHIFT)

#if defined(X86_32)

#define X2V_LARGE_PAGE_SIZE _4MB

#else

#define X2V_LARGE_PAGE_SIZE _2MB

#endif

//
// Number of pages for initial X2V stack.
//

#define X2V_STACK_PAGES     1

//
// Size of X2V stack in bytes.
//

#define X2V_STACK_SIZE      (X2V_STACK_PAGES * X2V_PAGE_SIZE)

#define X2V_TIMER_VECTOR    0x70
#define X2V_SPURIOUS_VECTOR 0x20

//
// 10 ms in nanoseconds.
//

#define X2V_10MS            ((UINT)10000000)

#define MAX_X2V_CPUS        32

//
// Linux vectors.
//

#define X2V_FIRST_LINUX_VECTOR      ((UINT)0x30)
#define X2V_LAST_LINUX_VECTOR   	((UINT)0x6F)

#if defined(X86_PAE)

#define X2V_XEN_VERSION_STRING      "xen-3.0-x86_32p"

#elif defined(X86_32)

#define X2V_XEN_VERSION_STRING      "xen-3.0-x86_32"

#elif defined(X86_64)

#define X2V_XEN_VERSION_STRING      "xen-3.0-x86_64"

#else

#define X2V_XEN_VERSION_STRING      "xen-3.0-x86_xx"

#endif

#define X2V_GLOBAL_PAGES            PDE_GLOBAL  
//#define X2V_GLOBAL_PAGES          0  

#endif  /* __HV_CONFIG_H__ */

