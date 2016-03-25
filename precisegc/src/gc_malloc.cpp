#include "gc_malloc.h"

ofstream _GC_::myfile;
_GC_::SegregatedList _GC_::segregated_storage[SegregatedStorageSize];

/* returns log_2 (size) if size is a power of 2
   otherwise --- -1 */
int _GC_::power_of_two (size_t size) {
	if ((size & (size -1)) != 0 || size == 0) { return -1; }
	int l = sizeof(size_t) * CHAR_BIT, r = 0;
	while (true) {
		int m = ((l - r) >> 1) + r;
		if (l == m || r == m) { return m; }
		if (((((size_t)1 << m) - 1) & size) == 0) { r = m; }
		else { l = m; }
	}
}

int _GC_::occuped_virtual_pages (size_t size) {
	if ((size & (size -1)) != 0 || size < MemoryCell) { return -1; }
	return (int)(size >> CellBits);
}

/* needs one more byte at the end of object */
/* align size to the closest greater power of two */
size_t _GC_::align_to_power_of_two (size_t size) {
	assert(size > 0);
	size_t i = size & (size -1);
	// if size === power of two then return it
	if (i == 0) { return size; }
	// otherwise --- round it up
	while (i != 0) { size = i; i = size & (size -1); }
	return size << 1;
}

/* align size to be a power of two and to be a multiple of MemoryCell */
size_t _GC_::align_object_size (size_t size) {
	assert(size > 0);
	size_t i = size & (size -1);
	// if size === power of two then return it
	if (i == 0) { return size /*> MemoryCell ? size : MemoryCell */ ; }
	// otherwise --- round it up
	while (i != 0) { size = i; i = size & (size -1); }
	size = size << 1;
	return size /* > MemoryCell ? size : MemoryCell */;
}
