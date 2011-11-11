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
 * HyperV operating system dependent header file
 *
 *****************************************************************************/

#ifndef __HV_OSD_H__
#define __HV_OSD_H__

/* Fixme -- added these for FreeBSD build */
#define do_cpuid x2v_do_cpuid  /* kludge to avoid namespace collision */
#ifndef __x86_64__
#define __x86_64__  
#endif

//
// Defines
//

#ifndef PAGE_SIZE
#define PAGE_SIZE 0x1000
#endif

#ifndef PAGE_SHIFT
#define PAGE_SHIFT 12
#endif

#define MAX_PAGE_BUFFER_COUNT 16
#define HW_MACADDR_LEN          6



#define STRUCT_PACKED		__attribute__((__packed__))
#define STRUCT_ALIGNED(x)	__attribute__((__aligned__(x)))

#define UNUSED_VAR(v)		v  __attribute__((__unused__))

#define ALIGN_UP(value, align)			( ((value) & (align-1))? ( ((value) + (align-1)) & ~(align-1) ): (value) )
#define ALIGN_DOWN(value, align)		( (value) & ~(align-1) )
#define NUM_PAGES_SPANNED(addr, len)	( (ALIGN_UP(addr+len, PAGE_SIZE) - ALIGN_DOWN(addr, PAGE_SIZE)) >> PAGE_SHIFT )

#ifndef MIN
#define MIN(a, b)       ((a) < (b)? (a): (b))
#endif

#ifndef MAX
#define MAX(a, b)       ((a) > (b)? (a): (b))
#endif

#define LOWORD(dw)		((unsigned short) (dw))
#define HIWORD(dw)		((unsigned short) (((unsigned int) (dw) >> 16) & 0xFFFF))

#define FIELD_OFFSET(t, f)    ((unsigned int)(unsigned long)&(((t *)0)->f))

#ifdef FALSE
#undef FALSE
#endif
#define FALSE 0

#ifdef TRUE
#undef TRUE
#endif
#define TRUE  1

#ifndef NULL
#define NULL  (void *)0
#endif

#ifndef INTERNAL
// Fixme
//#define INTERNAL static
#define INTERNAL extern
#endif

typedef struct _DLIST_ENTRY {
   struct _DLIST_ENTRY *Flink;
   struct _DLIST_ENTRY *Blink;
} DLIST_ENTRY;

//
// unsigned types
//
typedef unsigned char		UINT8;
typedef unsigned short		UINT16;
typedef unsigned int		UINT32;
#ifdef __x86_64__
typedef unsigned long		UINT64;
#else
typedef unsigned long long	UINT64;
#endif

typedef unsigned long long	ULONGLONG;
typedef unsigned int		ULONG;
typedef unsigned short		USHORT;
typedef unsigned char		UCHAR;

//
// signed types
//
typedef char				INT8;
typedef short				INT16;
typedef int					INT32;
#ifdef __x86_64__
typedef long				INT64;
#else
typedef long long		INT64;
#endif

typedef int					LONG;
typedef char				CHAR;
typedef long long			LONGLONG;

//
// Other types
//
typedef unsigned long		SIZE_T;
typedef void				VOID;
//typedef unsigned char		GUID[16];
typedef void*				PVOID;
typedef unsigned char		BOOL;
typedef unsigned char		BOOLEAN;
typedef void*				HANDLE;
typedef UINT32				DWORD;
typedef char*				PCHAR;
typedef unsigned char		BYTE;

typedef unsigned long		ULONG_PTR;

typedef struct {
	unsigned char	Data[16];
} GUID;

typedef void (*PFN_WORKITEM_CALLBACK)(void* context);
typedef void (*PFN_TIMER_CALLBACK)(void* context);

typedef UINT64 winfiletime_t; /* Windows FILETIME type */

#ifdef __x86_64__

#define RDMSR(reg, v) {                                                        \
    UINT32 h, l;                                                                 \
     __asm__ __volatile__("rdmsr"                                                               \
    : "=a" (l), "=d" (h)                                                       \
    : "c" (reg));                                                              \
    v = (((UINT64)h) << 32) | l;                                                         \
}

#define WRMSR(reg, v) {                                                        \
    UINT32 h, l;                                                               \
    l = (UINT32)(((UINT64)(v)) & 0xFFFFFFFF);                                  \
    h = (UINT32)((((UINT64)(v)) >> 32) & 0xFFFFFFFF);                          \
     __asm__ __volatile__("wrmsr"                                              \
    : /* no outputs */                                                         \
    : "c" (reg), "a" (l), "d" (h));                                            \
}

#else

#define RDMSR(reg, v) 			                                               \
     __asm__ __volatile__("rdmsr" 	                                           \
    : "=A" (v) 			                                                       \
    : "c" (reg))

#define WRMSR(reg, v) 			                                               \
     __asm__ __volatile__("wrmsr" 	                                           \
    : /* no outputs */ 				                                           \
    : "c" (reg), "A" ((UINT64)v))

#endif


//#ifdef REMOVED
/*
 * Fixme:  This confilics with a definition in /usr/include/machine/cpufunc.h
 * Fixme:  Prototype is different than this one.
 */
static inline void do_cpuid(unsigned int op, unsigned int *eax, unsigned int *ebx, unsigned int *ecx, unsigned int *edx)
{
	__asm__ __volatile__("cpuid" : "=a" (*eax), "=b" (*ebx), "=c" (*ecx), "=d" (*edx) : "0" (op), "c" (ecx));
}
//#endif

//
// operating system dependent routines
//
#ifdef REMOVED
/* Fixme:  Made a static inline in hv_logging.h for now */
extern void LogMsg(const char *fmt, ...);
#endif
/* Fixme:  Added this to keep compiler from hyperventilating */
static inline int LogMsg(const char *fmt, ...);

extern void BitSet(unsigned int* addr, int value);
extern void BitClear(unsigned int* addr, int value);
extern int BitTest(unsigned int* addr, int value);
extern int BitTestAndClear(unsigned int* addr, int value);
extern int BitTestAndSet(unsigned int* addr, int value);

extern int InterlockedIncrement(int *val);
extern int InterlockedDecrement(int *val);
extern int InterlockedCompareExchange(int *val, int new, int curr);

extern void Sleep(unsigned long usecs);

extern void* VirtualAllocExec(unsigned int size);
extern void VirtualFree(void* VirtAddr);

extern void* PageAlloc(unsigned int count);
extern void PageFree(void* page, unsigned int count);

extern void* MemMapIO(unsigned long phys, unsigned long size);
extern void MemUnmapIO(void* virt);

extern void* MemAlloc(unsigned int size);
extern void* MemAllocZeroed(unsigned int size);
extern void* MemAllocAtomic(unsigned int size);
extern void MemFree(void* buf);
extern void MemoryFence(VOID);

extern HANDLE TimerCreate(PFN_TIMER_CALLBACK pfnTimerCB, void* context);
extern void TimerClose(HANDLE hTimer);
extern int TimerStop(HANDLE hTimer);
extern void TimerStart(HANDLE hTimer, UINT32 expirationInUs);
extern SIZE_T GetTickCount(void);

extern void adj_guesttime(winfiletime_t hosttime, UINT8 flags);

extern HANDLE WaitEventCreate(void);
extern void WaitEventClose(HANDLE hWait);
extern void WaitEventSet(HANDLE hWait);
extern int	WaitEventWait(HANDLE hWait);

// If >0, hWait got signaled. If ==0, timeout. If < 0, error
extern int	WaitEventWaitEx(HANDLE hWait, UINT32 TimeoutInMs);

extern HANDLE SpinlockCreate(void);
extern void SpinlockClose(HANDLE hSpin);
extern void SpinlockAcquire(HANDLE hSpin);
extern void	SpinlockRelease(HANDLE hSpin);


#define GetVirtualAddress Physical2LogicalAddr
void* Physical2LogicalAddr(ULONG_PTR PhysAddr);

#define GetPhysicalAddress Logical2PhysicalAddr
ULONG_PTR Logical2PhysicalAddr(PVOID LogicalAddr);

ULONG_PTR Virtual2Physical(PVOID VirtAddr);

void* PageMapVirtualAddress(unsigned long Pfn);
void PageUnmapVirtualAddress(void* VirtAddr);


extern HANDLE WorkQueueCreate(char* name);
extern void WorkQueueClose(HANDLE hWorkQueue);
extern int WorkQueueQueueWorkItem(HANDLE hWorkQueue, PFN_WORKITEM_CALLBACK workItem, void* context);

extern void QueueWorkItem(PFN_WORKITEM_CALLBACK workItem, void* context);

extern int getCpuId(void);

extern int doOnAllCpus(void (*func) (void *info), void *info,
                              int retry, int wait);

extern void* PageAllocAtomic(unsigned int);

// extern void shutdown_onchannelcallback(void *context);


#endif  /* __HV_OSD_H__ */

