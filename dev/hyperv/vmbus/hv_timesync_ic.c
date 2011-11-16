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
 * HyperV channel timesync code
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

#ifdef REMOVED
#include "timesync_ic.h"
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
#include "hv_connection.h"
#include "hv_channel.h"
#include "hv_channel_interface.h"
#include "hv_ic.h"
// Fixme:  need this?  Was in hv_vmbus_private.h
#include "hv_timesync_ic.h"
#include "hv_vmbus_private.h"


void timesync_channel_cb(void *context){
        VMBUS_CHANNEL *channel = context;
	u8 *buf;
	u32 buflen, recvlen;
	u64 requestid;
	struct icmsg_hdr *icmsghdrp;
	struct icmsg_negotiate *negop;
	struct ictimesync_data *timedatap;

	DPRINT_ENTER(VMBUS);

	buflen = PAGE_SIZE;
	buf = MemAllocAtomic(buflen);

	VmbusChannelRecvPacket(channel, buf, buflen, &recvlen, &requestid);

	if(recvlen > 0) {
	    DPRINT_DBG(VMBUS, "timesync packet: recvlen=%d, requestid=%ld", 
			recvlen, requestid);

	    icmsghdrp = (struct icmsg_hdr *)&buf[sizeof(struct vmbuspipe_hdr)];
	    

	    if(icmsghdrp->icmsgtype == ICMSGTYPE_NEGOTIATE) {
		icmsghdrp->icmsgsize = 0x10;
		negop = (struct icmsg_negotiate *)&buf[
		    sizeof(struct vmbuspipe_hdr) +
		    sizeof(struct icmsg_hdr)];
		if(negop->icframe_vercnt == 2 &&
		   negop->icversion_data[1].major == 3) {
		    negop->icversion_data[0].major = 3;
		    negop->icversion_data[0].minor = 0;
		    negop->icversion_data[1].major = 3;
		    negop->icversion_data[1].minor = 0;
		} else {
		    negop->icversion_data[0].major = 1;
		    negop->icversion_data[0].minor = 0;
		    negop->icversion_data[1].major = 1;
		    negop->icversion_data[1].minor = 0;
		}
		negop->icframe_vercnt = 1;
		negop->icmsg_vercnt = 1;
	    } else {
		timedatap = (struct ictimesync_data *)&buf[
		    sizeof(struct vmbuspipe_hdr) +
		    sizeof(struct icmsg_hdr)];
		adj_guesttime(timedatap->parenttime, timedatap->flags);
	    }

	    icmsghdrp->icflags = ICMSGHDRFLAG_TRANSACTION 
		| ICMSGHDRFLAG_RESPONSE;

	    VmbusChannelSendPacket(channel, buf,
				   recvlen, requestid,
				   VmbusPacketTypeDataInBand, 0);
	}

	MemFree(buf);

	DPRINT_EXIT(VMBUS);
}

