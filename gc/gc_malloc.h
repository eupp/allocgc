#pragma once
#include <assert.h>
#include <unistd.h>
#include "malloc.h"
#include <stdlib.h>
#include <vector>
#include <sys/mman.h>
#include <limits.h>
#include <fstream>
#include <string.h>

using namespace std;

#ifndef _GC_CELL_BITS
#define _GC_CELL_BITS 12
#endif

#define _GC_SYSTEM_BIT_SIZE 64
#if ((_GC_CELL_BITS) < 1) || ((_GC_CELL_BITS) > ((_GC_SYSTEM_BIT_SIZE) - 2))
#error GC_CELL_BITS out of range
#endif

#ifndef _GC_SMALL_BIT_ALIGN
#define _GC_SMALL_BIT_ALIGN ((size_t)3)
#endif

#ifndef OBJECTS_PER_PAGE_BITS
#define OBJECTS_PER_PAGE_BITS    ((size_t)5)
#endif

#ifndef SSE_SIZE
#define SSE_SIZE 4096
#endif

namespace _GC_ {
    extern ofstream myfile;

    struct PageDescriptor; struct SegregatedListElement;
	enum {
		CellBits                = _GC_CELL_BITS,
		MemoryCell              = 1 << _GC_CELL_BITS,
		SystemBitSize           =_GC_SYSTEM_BIT_SIZE,
		SmallBitAlign           = _GC_SMALL_BIT_ALIGN,
		ObjectsPerPageBits      = OBJECTS_PER_PAGE_BITS,
		SegregatedStorageSize   = (_GC_SYSTEM_BIT_SIZE - _GC_SMALL_BIT_ALIGN - 1),
		ObjectsPerPage          = (1 << OBJECTS_PER_PAGE_BITS),
		BitsPerLong             = (sizeof(long) * CHAR_BIT),
		MarkBitsArraySize       = (ObjectsPerPage / BitsPerLong + 1),
		SSESize                 = SSE_SIZE
	};

    // segregated storage page header
    /// its size might be aligned on power of two
    /* 2^OBJECTS_PER_PAGE_BITS equals to the count of objects on page (by default; can de changed) */
    struct PageDescriptor {
	    size_t obj_size; // sizeof object (whole object (ex. in array of case --- whole array) with a header)
	    size_t page_size;
	    size_t mask; // a mask for pointers that points on this page (is used to find object begin)
	    void *free; // pointer on the next after the last allocated Object. If Page is full --- NULL
	    void *page; // pointer on the page itself
	    long mark_bits[MarkBitsArraySize]; //mark bits for objects in
	    long pin_bits[MarkBitsArraySize]; //pin bits for objects in
    };

    struct SegregatedListElement {
	    SegregatedListElement *next, *prev;
	    uint last_descr; // last descr of last allocated page
	    PageDescriptor descrs[0];
    };
    struct SegregatedList {
	    uint size; // log_2 (sizeof objects that are stored in this Segregated List)
	    SegregatedListElement *first, *last; // pointer on the first SegregatedListElement
    };
    extern SegregatedList segregated_storage[SegregatedStorageSize];

	enum {
		SSEDescrCount = ((SSE_SIZE - sizeof(SegregatedListElement)) / sizeof(PageDescriptor))
	};

	// header of each object in the heap (stored at the end of the object space)
	/// its size might be aligned on power of two
    struct Object {
	    //	first to fields together === base_meta
	    void *meta; // pointer on the meta information
	    size_t count;
	    void *begin; // pointer on th object begin
	    // NB: FALSE in current version: we use last two bits of begin pointer to be a pin and mask bits respectively
    };

    int power_of_two(size_t size);

    int occuped_virtual_pages(size_t size);

    size_t align_to_power_of_two(size_t size);

    size_t align_object_size(size_t size);
}