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
 * HyperV vmbus network vsc file
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


#ifndef __HV_RNDIS_FILTER_H__
#define __HV_RNDIS_FILTER_H__


#define __struct_bcount(x)

#ifdef REMOVED
/* Fixme:  Removed */
#include "osd.h"
#include "NetVsc.h"

#include "rndis.h"
#endif

#define RNDIS_HEADER_SIZE	(sizeof(RNDIS_MESSAGE) - sizeof(RNDIS_MESSAGE_CONTAINER))

#define NDIS_PACKET_TYPE_DIRECTED	0x00000001
#define NDIS_PACKET_TYPE_MULTICAST	0x00000002
#define NDIS_PACKET_TYPE_ALL_MULTICAST	0x00000004
#define NDIS_PACKET_TYPE_BROADCAST	0x00000008
#define NDIS_PACKET_TYPE_SOURCE_ROUTING	0x00000010
#define NDIS_PACKET_TYPE_PROMISCUOUS	0x00000020
#define NDIS_PACKET_TYPE_SMT		0x00000040
#define NDIS_PACKET_TYPE_ALL_LOCAL	0x00000080
#define NDIS_PACKET_TYPE_GROUP		0x00000100
#define NDIS_PACKET_TYPE_ALL_FUNCTIONAL	0x00000200
#define NDIS_PACKET_TYPE_FUNCTIONAL	0x00000400
#define NDIS_PACKET_TYPE_MAC_FRAME	0x00000800



//
// Interface
//
int
RndisFilterInit(
	NETVSC_DRIVER_OBJECT *Driver
	);

/*
 * Externs
 */
extern int RndisSetMode(DEVICE_OBJECT *Device, int mode);


#endif  /* __HV_RNDIS_FILTER_H__ */

