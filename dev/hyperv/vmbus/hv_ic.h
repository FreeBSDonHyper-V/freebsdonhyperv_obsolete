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
 * HyperV common header for Hyper-V ICs
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
#ifndef _IC_H_
#define _IC_H_

/**
 * Common header for Hyper-V ICs
 */

typedef UINT8 u8;
typedef UINT16 u16;
typedef UINT32 u32;
typedef UINT64 u64;

#define ICMSGTYPE_NEGOTIATE 0
#define ICMSGTYPE_HEARTBEAT 1
#define ICMSGTYPE_KVPEXCHANGE 2
#define ICMSGTYPE_SHUTDOWN 3
#define ICMSGTYPE_TIMESYNC 4
#define ICMSGTYPE_VSS 5

#define ICMSGHDRFLAG_TRANSACTION 1
#define ICMSGHDRFLAG_REQUEST 2
#define ICMSGHDRFLAG_RESPONSE 4

struct vmbuspipe_hdr{
    u32 flags;
    u32 msgsize;
} __attribute__((packed));

struct ic_version {
    u16 major;
    u16 minor;
} __attribute__((packed));

struct icmsg_hdr {
    struct ic_version icverframe;
    u16 icmsgtype;
    struct ic_version icvermsg;
    u16 icmsgsize;
    u32 status;
    u8 ictransaction_id;
    u8 icflags;
    u8 reserved[2];
} __attribute__((packed));   

struct icmsg_negotiate{
    u16 icframe_vercnt;
    u16 icmsg_vercnt;
    u32 reserved;
    struct ic_version icversion_data[1]; /* any size array */
} __attribute__((packed));

struct shutdown_msg_data
{
        u32 reason_code;
        u32 timeout_seconds;
        u32 flags;
        u8  display_message[2048];
} __attribute__((packed));

struct heartbeat_msg_data
{
	u64 seq_num;
	u32 reserved[8];
} __attribute__((packed));


#endif  /* _IC_H_ */

