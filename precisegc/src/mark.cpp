#include "gc_malloc.h"
#include "index_tree.h"
//////////
// MARK //
//////////

extern size_t marked_objects;

using namespace _GC_;

PageDescriptor * calculate_ob_i_and_mask (size_t * ob_i, long * mask, void * ptr) {
	PageDescriptor * d = (PageDescriptor *)IT_get_page_descr(ptr);
	if (d == NULL) { return NULL; }
	int pot = power_of_two(d->obj_size), bi = (int)SystemBitSize - (int)ObjectsPerPageBits - pot;
	*ob_i = ((d->mask & (size_t)ptr) << bi) >> (bi + pot);
//	assert(ptr == d->page + *ob_i * d->obj_size);
	assert(ptr >= d->page + *ob_i * d->obj_size &&
		   ptr <= d->page + (*ob_i + 1) * d->obj_size);
	size_t bit_i = *ob_i % BitsPerLong;
	*mask = (long)1 << (BitsPerLong - bit_i - 1);
	return d;
}

namespace _GC_
{

/*
mark_pin --- false => pin bit; true => mark_bit
return values:
	 0 --- successfully marked
	 1 --- already marked
	-1 --- incorrect pointer(not in the heap) */
int mark_object (void * ptr, bool mark_pin) {
	// ob_i --- calculate object number on the page
	size_t ob_i; long mask; PageDescriptor * d = calculate_ob_i_and_mask(&ob_i, &mask, ptr);
	assert(d != NULL);
//    if (d == NULL) { return -1; }
	long * array = mark_pin ? d->mark_bits : d->pin_bits;
	assert(mask != 0);
	if ((array[ob_i / BitsPerLong] & mask) == mask) { return 1; }
	array[ob_i / BitsPerLong] |= mask;
	if (mark_pin) marked_objects++;
	return 0;

}

}

int clear_object_bit (void * ptr, bool mark_pin) {
	size_t ob_i; long mask; PageDescriptor * d = calculate_ob_i_and_mask(&ob_i, &mask, ptr);
	if (d == NULL) { return -1; }
	long * array = mark_pin ? d->mark_bits : d->pin_bits;
	array[ob_i / BitsPerLong] &= ~mask;
	return 0;
}

int clear_all_object_bits (void * ptr) {
	size_t ob_i; long mask; PageDescriptor * d = calculate_ob_i_and_mask(&ob_i, &mask, ptr);
	if (d == NULL) { return -1; }
	d->mark_bits[ob_i / BitsPerLong] &= ~mask; d->pin_bits[ob_i / BitsPerLong] &= ~mask;
	return 0;
}

namespace _GC_ {

int get_object_mark(void *ptr, bool mark_pin) {
    size_t ob_i;
    long mask;
    PageDescriptor *d = calculate_ob_i_and_mask(&ob_i, &mask, ptr);
    if (d == NULL) { return -1; }
    long *array = mark_pin ? d->mark_bits : d->pin_bits;
    return (array[ob_i / BitsPerLong] & mask) == mask;
}

bool mark_after_overflow() {
// TODO write it
//    myfile << "mark_after_overflow is called! ERROR! Exit!" << endl;
    assert(true);
    return true;
}

}

void clear_page_flags (_GC_::PageDescriptor * d) {
    if (d == NULL) { return; }
    memset(d->mark_bits, 0, _GC_::MarkBitsArraySize * sizeof(long));
    memset(d->pin_bits, 0, _GC_::MarkBitsArraySize * sizeof(long));
}