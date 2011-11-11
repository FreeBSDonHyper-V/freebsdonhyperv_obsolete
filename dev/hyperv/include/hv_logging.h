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
 * HyperV logging header file
 *
 *****************************************************************************/

#ifndef __HV_LOGGING_H__
#define __HV_LOGGING_H__

//#include <linux/init.h>
//#include <linux/module.h>

#define DEBUG

#ifdef REMOVED
/* Fixme -- removed */
#include "osd.h"
#endif

#define VMBUS				0x0001
#define STORVSC				0x0002
#define NETVSC				0x0004
#define INPUTVSC			0x0008
#define BLKVSC				0x0010
#define VMBUS_DRV			0x0100
#define STORVSC_DRV			0x0200
#define NETVSC_DRV			0x0400
#define INPUTVSC_DRV		0x0800
#define BLKVSC_DRV			0x1000

#define ALL_MODULES			(VMBUS		|\
							STORVSC		|\
							NETVSC		|\
							INPUTVSC	|\
							BLKVSC		|\
							VMBUS_DRV	|\
							STORVSC_DRV	|\
							NETVSC_DRV	|\
							INPUTVSC_DRV|\
							BLKVSC_DRV) 

// Logging Level
#define CRITICAL_LVL				2
#define ERROR_LVL					3
#define WARNING_LVL					4
#define INFO_LVL					6
#define DEBUG_LVL					7
#define DEBUG_LVL_ENTEREXIT			8
#define DEBUG_RING_LVL				9

extern unsigned int vmbus_loglevel;

#define ASSERT(expr)	\
        if (!(expr)) {	\
		LogMsg("<%d>Assertion failed! %s,%s,%s,line=%d\n", CRITICAL_LVL, #expr,__FILE__,__FUNCTION__,__LINE__);	\
		__asm__ __volatile__("int3");	\
        }

#define DPRINT(mod, lvl, fmt, args...) do {\
	if (mod & (HIWORD(vmbus_loglevel))) \
		(lvl <= LOWORD(vmbus_loglevel))?(LogMsg("<%d>" #mod": %s() " fmt "\n", DEBUG_LVL, __FUNCTION__, ## args)):(0);\
	} while (0)

#define DPRINT_DBG(mod, fmt, args...) do {\
	if (mod & (HIWORD(vmbus_loglevel))) \
		(DEBUG_LVL <= LOWORD(vmbus_loglevel))?(LogMsg("<%d>" #mod": %s() " fmt "\n", DEBUG_LVL, __FUNCTION__, ## args)):(0);\
	} while (0)

#define DPRINT_INFO(mod, fmt, args...) do {\
	if (mod & (HIWORD(vmbus_loglevel))) \
		(INFO_LVL <= LOWORD(vmbus_loglevel))?(LogMsg("<%d>" #mod": " fmt "\n", INFO_LVL, ## args)):(0);\
	} while (0)

#define DPRINT_WARN(mod, fmt, args...) do {\
	if (mod & (HIWORD(vmbus_loglevel))) \
		(WARNING_LVL <= LOWORD(vmbus_loglevel))?(LogMsg("<%d>" #mod": WARNING! " fmt "\n", WARNING_LVL, ## args)):(0);\
	} while (0)

// Fixme
#define DPRINT_ERR(mod, fmt, args...) do {\
	if (mod & (HIWORD(vmbus_loglevel))) \
		(ERROR_LVL <= LOWORD(vmbus_loglevel))?(LogMsg("<%d>" #mod": %s() ERROR!! " fmt "\n", ERROR_LVL, __FUNCTION__, ## args)):(0);\
	} while (0)

#ifdef DEBUG
#define DPRINT_ENTER(mod) do {\
	if (mod & (HIWORD(vmbus_loglevel))) \
		(DEBUG_LVL_ENTEREXIT <= LOWORD(vmbus_loglevel))?(LogMsg("<%d>" "["#mod"]: %s() enter\n", DEBUG_LVL, __FUNCTION__)):(0);\
	} while (0)

#define DPRINT_EXIT(mod) do {\
	if (mod & (HIWORD(vmbus_loglevel))) \
		(DEBUG_LVL_ENTEREXIT <= LOWORD(vmbus_loglevel))?(LogMsg("<%d>" "["#mod"]: %s() exit\n", DEBUG_LVL, __FUNCTION__)):(0);\
	} while (0)
#else
#define DPRINT_ENTER(mod)
#define DPRINT_EXIT(mod)
#endif

static inline void PrintBytes(const unsigned char* bytes, int len)
{
	int i=0;

	LogMsg("\n<< ");
	for (i=0; i< len; i++)
	{
		LogMsg("0x%x ", bytes[i]);
	}
	LogMsg(">>\n");
}

//
// Inline
//
//static inline void GuidToStr(const GUID g, char *str)
//{
//	sprintf(str, "{%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x%02x%02x}", 
//	g[3], g[2], g[1], g[0], g[5], g[4], g[7], g[6], g[8], g[9], g[10], g[11], g[12], g[13], g[14], g[15]);
//
//}

/*
 * Fixme:  Replaces NetScaler code in usr.src/sys/kern/subr_prf.c
 * Fixme:  Currently disabled
 * Fixme:  Probably ought to be a real function.  Where to put it?
 */
static inline int LogMsg(const char *fmt, ...)
{
	int retval = 0;
#ifdef REMOVED
	va_list ap;
	int savintr;
	struct putchar_arg pca;

	savintr = consintr;             /* disable interrupts */
	consintr = 0;
	va_start(ap, fmt);
	pca.tty = NULL;
	pca.flags = TOCONS | TOLOG | TONS; // NETSCALER
	pca.pri = -1;
	retval = kvprintf(fmt, putchar, &pca, 10, ap);
	va_end(ap);
ifdef NETSCALER
ifndef VMPE_KERN
	ns_conmsgappwakeup();
endif
endif
	if (!panicstr)
		msgbuftrigger = 1;
	consintr = savintr;             /* reenable interrupts */
#endif

	return (retval);
}



#endif  /* __HV_LOGGING_H__ */
