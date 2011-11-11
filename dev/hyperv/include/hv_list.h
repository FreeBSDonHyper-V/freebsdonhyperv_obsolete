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
 * HyperV singly and doubly linked list implementation using macros.
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

#ifndef __HV_LIST_H__
#define __HV_LIST_H__

#ifdef REMOVED
/* Fixme:  Removed */
#include "osd.h"
#endif

/*
 *
 *  Doubly-linked list manipulation routines.  Implemented as macros
 *  but logically these are procedures.
 *
 */

typedef DLIST_ENTRY LIST_ENTRY;
typedef DLIST_ENTRY *PLIST_ENTRY;

//typedef struct LIST_ENTRY {
//   struct LIST_ENTRY * volatile Flink;
//   struct LIST_ENTRY * volatile Blink;
//} LIST_ENTRY, *PLIST_ENTRY;



/*
 *  VOID
 *  InitializeListHead(
 *      PLIST_ENTRY ListHead
 *      );
 */
#define INITIALIZE_LIST_HEAD	InitializeListHead

#define InitializeListHead(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = (ListHead))


/*
 *  BOOLEAN
 *  IsListEmpty(
 *      PLIST_ENTRY ListHead
 *      );
 */
#define IS_LIST_EMPTY			IsListEmpty

#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))


/*
 *  PLIST_ENTRY
 *  NextListEntry(
 *      PLIST_ENTRY Entry
 *      );
 */
#define	NEXT_LIST_ENTRY			NextListEntry

#define NextListEntry(Entry) \
    (Entry)->Flink


/*
 *  PLIST_ENTRY
 *  PrevListEntry(
 *      PLIST_ENTRY Entry
 *      );
 */
#define	PREV_LIST_ENTRY			PrevListEntry

#define PrevListEntry(Entry) \
    (Entry)->Blink


/*
 *  PLIST_ENTRY
 *  TopListEntry(
 *      PLIST_ENTRY ListHead
 *      );
 */
#define	TOP_LIST_ENTRY			TopListEntry

#define TopListEntry(ListHead) \
    (ListHead)->Flink



/*
 *  PLIST_ENTRY
 *  RemoveHeadList(
 *      PLIST_ENTRY ListHead
 *      );
 */

#define	REMOVE_HEAD_LIST		RemoveHeadList

#define RemoveHeadList(ListHead) \
    (ListHead)->Flink;\
    {RemoveEntryList((ListHead)->Flink)}


/*
 *  PLIST_ENTRY
 *  RemoveTailList(
 *      PLIST_ENTRY ListHead
 *      );
 */
#define	REMOVE_TAIL_LIST		RemoveTailList

#define RemoveTailList(ListHead) \
    (ListHead)->Blink;\
    {RemoveEntryList((ListHead)->Blink)}


/*
 *  VOID
 *  RemoveEntryList(
 *      PLIST_ENTRY Entry
 *      );
 */
#define	REMOVE_ENTRY_LIST		RemoveEntryList

#define RemoveEntryList(Entry) {\
    PLIST_ENTRY _EX_Flink = (Entry)->Flink;\
    PLIST_ENTRY _EX_Blink = (Entry)->Blink;\
    _EX_Blink->Flink = _EX_Flink;\
    _EX_Flink->Blink = _EX_Blink;\
	}


/*
 *  VOID
 *  AttachList(
 *      PLIST_ENTRY ListHead,
 *      PLIST_ENTRY ListEntry
 *      );
 */
#define	ATTACH_LIST		AttachList

#define AttachList(ListHead,ListEntry) {\
    PLIST_ENTRY _EX_ListHead = (ListHead);\
    PLIST_ENTRY _EX_Blink = (ListHead)->Blink;\
    (ListEntry)->Blink->Flink = _EX_ListHead;\
    _EX_Blink->Flink = (ListEntry);\
    _EX_ListHead->Blink = (ListEntry)->Blink;\
    (ListEntry)->Blink = _EX_Blink;\
    }



/*
 *  VOID
 *  InsertTailList(
 *      PLIST_ENTRY ListHead,
 *      PLIST_ENTRY Entry
 *      );
 */

#define	INSERT_TAIL_LIST		InsertTailList

#define InsertTailList(ListHead,Entry) {\
    PLIST_ENTRY _EX_ListHead = (ListHead);\
    PLIST_ENTRY _EX_Blink = (ListHead)->Blink;\
    (Entry)->Flink = _EX_ListHead;\
    (Entry)->Blink = _EX_Blink;\
    _EX_Blink->Flink = (Entry);\
    _EX_ListHead->Blink = (Entry);\
    }


/*
 *  VOID
 *  InsertHeadList(
 *      PLIST_ENTRY ListHead,
 *      PLIST_ENTRY Entry
 *      );
 */
#define	INSERT_HEAD_LIST		InsertHeadList

#define InsertHeadList(ListHead,Entry) {\
    PLIST_ENTRY _EX_ListHead = (ListHead);\
    PLIST_ENTRY _EX_Flink = (ListHead)->Flink;\
    (Entry)->Flink = _EX_Flink;\
    (Entry)->Blink = _EX_ListHead;\
    _EX_Flink->Blink = (Entry);\
    _EX_ListHead->Flink = (Entry);\
    }


/*
 *  VOID
 *  IterateListEntries(
 *      PLIST_ENTRY anchor,
 *      PLIST_ENTRY index,
 *		PLIST_ENTRY listp
 *      );
 */

#define	ITERATE_LIST_ENTRIES	IterateListEntries

#define IterateListEntries(anchor, index, listp) \
	(anchor) = (LIST_ENTRY *)(listp); \
	for((index) = (anchor)->Flink; (index) != (anchor); (index) = (index)->Flink)



/*
 *  PSINGLE_LIST_ENTRY
 *  PopEntryList(
 *      PSINGLE_LIST_ENTRY ListHead
 *      );
 */

#define	POP_ENTRY_LIST		PopEntryList

#define PopEntryList(ListHead) \
    (ListHead)->Next;\
    {\
        PSINGLE_LIST_ENTRY FirstEntry;\
        FirstEntry = (ListHead)->Next;\
        if (FirstEntry != NULL) {     \
            (ListHead)->Next = FirstEntry->Next;\
        }                             \
    }



/*
 *  VOID
 *  PushEntryList(
 *      PSINGLE_LIST_ENTRY ListHead,
 *		PSINGLE_LIST_ENTRY Entry
 *      );
 */

#define	PUSH_ENTRY_LIST			PushEntryList

#define PushEntryList(ListHead,Entry) \
    (Entry)->Next = (ListHead)->Next; \
    (ListHead)->Next = (Entry)

#ifndef CONTAINING_RECORD
#define CONTAINING_RECORD(address, type, field) ((type *)( \
                                                  (PCHAR)(address) - \
                                                  (PCHAR)(&((type *)0)->field)))
#endif /* CONTAINING_RECORD */

#endif  /* __HV_LIST_H__ */

/* EOF */
