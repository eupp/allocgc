#include <malloc.h>
#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "malloc.h"
#include <stdlib.h>
#include <vector>
#include <sys/mman.h>
#include <limits.h>
#include <fstream>
#include <unistd.h>
#include <string.h>

using namespace std;

ofstream myfile;

#define VIRTUAL_MEMORY_CELL sysconf(_SC_PAGE_SIZE)
#define VIRTUAL_MEMORY_CELL_BITS power_of_two(VIRTUAL_MEMORY_CELL)
// int posix_memalign(void **memptr, size_t alignment, size_t size);
#define SYSTEM_BIT_SIZE ((size_t)64)
// #define SMALL_BIT_ALIGN VIRTUAL_MEMORY_CELL_BITS
#define SMALL_BIT_ALIGN ((size_t)3)
#define SEGREGATED_STORAGE_SIZE (SYSTEM_BIT_SIZE - SMALL_BIT_ALIGN - 1)

/* returns log_2 (size) if size is a power of 2
   otherise --- -1 */
// // previous version:
// int power_of_two (size_t size) {
// 	if ((size & (size -1)) != 0) { return -1; }
// 	int res = 0;
// 	for (; size != 0; size = size >> 1, res++) {}
// 	return res;
// }
int power_of_two (size_t size) {
	if ((size & (size -1)) != 0 || size == 0) { return -1; }
	int l = sizeof(size_t) * CHAR_BIT, r = 0;
	while (true) {
		int m = ((l - r) >> 1) + r;
		if (l == m || r == m) { return m; }
		if (((((size_t)1 << m) - 1) & size) == 0) { r = m; }
		else { l = m; }
	}
	return l;
}
int occuped_virtual_pages (size_t size) {
	if ((size & (size -1)) != 0 || size < VIRTUAL_MEMORY_CELL) { return -1; }
	return size >> VIRTUAL_MEMORY_CELL_BITS;
}

////////////////
// INDEX TREE //
////////////////

#define IT_LEVEL_COUNT			((size_t) 3)
#define IT_BITS_IN_USE			(SYSTEM_BIT_SIZE - VIRTUAL_MEMORY_CELL_BITS)
#define IT_FIRST_LEVEL_SIZE		(IT_BITS_IN_USE - (size_t)(IT_BITS_IN_USE / IT_LEVEL_COUNT) * (IT_LEVEL_COUNT - 1))
#define IT_OTHER_LEVELS_SIZE	((size_t)(IT_BITS_IN_USE / IT_LEVEL_COUNT))
size_t * tree_level_one = (size_t *)mmap(NULL, (((size_t) 2) << IT_FIRST_LEVEL_SIZE) * sizeof(size_t),
			PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);

void IT_index_new_cell (void * cell, void * page_decr) {
	size_t bits_curr = SYSTEM_BIT_SIZE - VIRTUAL_MEMORY_CELL_BITS - IT_FIRST_LEVEL_SIZE;
	size_t i_l = (size_t)cell >> bits_curr;
	size_t * level = tree_level_one;

	for (int i = 0; i < (int)IT_LEVEL_COUNT - 1; i++) {
		level[i_l] = (void *)level[i_l] != NULL ? level[i_l] : (size_t)mmap(NULL, (((size_t) 2) << IT_OTHER_LEVELS_SIZE) * sizeof(size_t),
			PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
		assert((void *)level[i_l] != NULL);
		level = reinterpret_cast <size_t *> (level[i_l]);
		bits_curr -= IT_OTHER_LEVELS_SIZE;
		i_l = ((size_t)cell >> (bits_curr + VIRTUAL_MEMORY_CELL_BITS)) & (((size_t)1 << (IT_OTHER_LEVELS_SIZE)) - 1);
	}
	level[i_l] = (size_t)page_decr;
}

size_t * IT_iterate (void * ptr, size_t * bits_curr, size_t * i_l, size_t * level) {
	for (int i = 0; i < (int)IT_LEVEL_COUNT - 1; i++) {
		if ((void *)level[*i_l] == NULL) { return NULL; }
		level = reinterpret_cast <size_t *> (level[*i_l]);
		*bits_curr -= IT_OTHER_LEVELS_SIZE;
		*i_l = ((size_t)ptr >> (*bits_curr + VIRTUAL_MEMORY_CELL_BITS)) & (((size_t)1 << (IT_OTHER_LEVELS_SIZE)) - 1);
	}
	return level;
}

/* return a page ptr points to descriptor */
void * IT_get_page_descr (void * ptr) {
	size_t bits_curr = SYSTEM_BIT_SIZE - VIRTUAL_MEMORY_CELL_BITS - IT_FIRST_LEVEL_SIZE;
	size_t i_l = (size_t)ptr >> bits_curr;
	size_t * level = IT_iterate(ptr, &bits_curr, &i_l, tree_level_one);
	return level != NULL ? (void *) level[i_l] : NULL;
}

/* remove index */
void IT_remove_index (void * cell) {
	size_t bits_curr = SYSTEM_BIT_SIZE - VIRTUAL_MEMORY_CELL_BITS - IT_FIRST_LEVEL_SIZE;
	size_t i_l = (size_t)cell >> bits_curr;
	size_t * level = IT_iterate(cell, &bits_curr, &i_l, tree_level_one);
	if (level == NULL) { myfile << "IT_remove_index : doesn't contain cell " << cell << endl; return; }
	level[i_l] = 0;
}

//////////////
// GCMALLOC //
//////////////

/* needs one more byte at the end of object */
// #define align_small_bits(ptr) ((ptr & ((1 << SMALL_BIT_ALIGN) - 1)) == 0 ? ptr : ((ptr >> SMALL_BIT_ALIGN) + 1) << SMALL_BIT_ALIGN)
// #define align(ptr, alignment) ((ptr & alignment) == 0 ? ptr : (((ptr >> alignment) + 1) << alignment))
/* align size to the closest greater power of two */
size_t align_to_power_of_two (size_t size) {
	assert(size > 0);
	int i = size & (size -1);
	// if size === power of two then return it
	if (i == 0) { return size; }
	// otherwise --- round it up
	while (i != 0) { size = i; i = size & (size -1); }
	return size << 1;
}
/* align size to be a power of two and to be a multiple of VIRTUAL_MEMORY_CELL */
size_t align_object_size (size_t size) {
	assert(size > 0);
	int i = size & (size -1);
	// if size === power of two then return it
	if (i == 0) { return size > VIRTUAL_MEMORY_CELL ? size : VIRTUAL_MEMORY_CELL; }
	// otherwise --- round it up
	while (i != 0) { size = i; i = size & (size -1); }
	size = size << 1;
	return size > VIRTUAL_MEMORY_CELL ? size : VIRTUAL_MEMORY_CELL;
}

// header of each object in the heap (stored at the end of the object space)
/// its size might be aligned on power of two
struct Object {
	size_t array_count;
	// NB! meta have to contains single object size
	void * meta; // pointer on the metainformation
	void * begin; // pointer on th object begin
	// we use last two bits of begin pointer to be a pin and mask bits respectively
};

// segregated storage page header
/// its size might be aligned on power of two
/* 2^OBJECTS_PER_PAGE_BITS equals to the count of objects on page (by default; can de changed) */
#define OBJECTS_PER_PAGE_BITS	((size_t)5)
#define OBJECTS_PER_PAGE_COUNT	(1 << OBJECTS_PER_PAGE_BITS - 1)
#define BITS_PER_LONG			(sizeof(long) * CHAR_BIT)
#define MARK_BITS_ARRAY_SIZE	(OBJECTS_PER_PAGE_COUNT / BITS_PER_LONG)
struct PageDescriptor {
	size_t obj_size; // sizeof object (whole object (ex. in array of case --- whole array) with a header)
	size_t page_size;
	size_t mask; // a mask for pointers that points on this page (is used to find object begin)
	void * free; // pointer on the next after the last allocated Object. If Page is full --- NULL
	void * page; // pointer on the page itself
	long mark_bits[MARK_BITS_ARRAY_SIZE]; //mark bits for objects in
	long pin_bits[MARK_BITS_ARRAY_SIZE]; //pin bits for objects in
};

// devided on obejct size
#define SSE_SIZE 4096
#define SSE_DESCR_COUNT ((SSE_SIZE - sizeof(SegregatedListElement)) / sizeof(PageDescriptor))
struct SegregatedListElement {
	SegregatedListElement * next;
	uint last_descr; // last descr of last allocated page
	PageDescriptor descrs[0];
};
struct SegregatedList {
	uint size; // log_2 (sizeof objects that are stored in this Segregated List)
	SegregatedListElement * first; // pointer on the first SegregatedListElement
};
SegregatedList segregated_storage[SEGREGATED_STORAGE_SIZE];

void init_segregated_storage (void) {
	myfile.open("log.out");

	size_t sle_size = 4; // i.e. min size == 32 (i.e. round_up_to_power_of_two(16(i.e. 16 === sizeof(Object)) + ?))
	for (int i = 0; i < SEGREGATED_STORAGE_SIZE; i++, sle_size++) {
		segregated_storage[i].size = sle_size;
		segregated_storage[i].first = NULL;
	}
}

SegregatedListElement * allocate_new_SegregatedListElement (int ss_i) {
	SegregatedListElement * sle = (SegregatedListElement *)mmap(NULL, SSE_SIZE,
		PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
	sle->next = segregated_storage[ss_i].first;
	sle->last_descr = 0;
	for (int i = 0; i < SSE_DESCR_COUNT; i++) { sle->descrs[i] = {0, 0, 0, NULL, NULL}; }
	segregated_storage[ss_i].first = sle;
	return sle;
}

size_t calculate_mask (size_t page_size, size_t obj_size, void * page_addr) {
	size_t page_count_bits = power_of_two(page_size), obj_size_bits = power_of_two(obj_size);
	size_t bit_diff = page_count_bits - obj_size_bits;
	assert(page_count_bits != -1 && obj_size_bits != -1);
	return ((size_t)page_addr | ((1 << bit_diff) - 1) << obj_size_bits);
}

void * index_new_page (PageDescriptor * d) {
	size_t page = (size_t)d->page, size = d->page_size;
	for (void * i = (void *)page; (size_t)i - page < size; i = (void *)((size_t)i + VIRTUAL_MEMORY_CELL)) {
		IT_index_new_cell(i, d);
	}
}

void * allocate_on_new_page (PageDescriptor * d, size_t obj_size, void * meta, size_t count) {
	// i.e. it is no space (following our allocating stotegy)
	// so allocate new page
	myfile << "\tallocate_on_new_page" << endl;
	size_t objects_on_page_bits = OBJECTS_PER_PAGE_BITS;
	assert(power_of_two(obj_size) + objects_on_page_bits < 64 - VIRTUAL_MEMORY_CELL_BITS);
	size_t page_size = obj_size << objects_on_page_bits;
	void * new_page = NULL;
	while (new_page == NULL) {
		assert(objects_on_page_bits < 16); // can fails there
		assert(page_size == (obj_size << objects_on_page_bits));
		// NB! if insted of memalign(page_size, page_size) call memalign(VIRTUAL_MEMORY_CELL, page_size);  WILL BE BUG!!!!
		new_page = (void *)memalign(page_size, page_size);
		if (new_page == NULL) {
			page_size = (page_size << 1); objects_on_page_bits++;
		} else {
			myfile << "\tmemalign: begin = " << new_page << " end = " << (void *)((size_t)new_page + page_size) << " page_size = " << page_size << endl;
		}
	}
	d->page = new_page;
	d->obj_size = obj_size;
	d->page_size = page_size;
	d->free = (void *)((size_t)d->page + d->obj_size);
	d->mask = calculate_mask(page_size, obj_size, new_page);
	myfile << "\t\t res = page = " << d->page << " free = " << d->free << " obj_size = " << d->obj_size << " page_size = " << d->page_size << " mask = " << d->mask << endl;
	// add new cells to index tree
	index_new_page(d);

	void * res = new_page;
	Object * obj = (Object *)((size_t)res + obj_size - sizeof(Object));
	obj->array_count = count;
	obj->meta = meta;
	obj->begin = res;
	myfile << "allocate_on_new_page: end\n" << endl;
	return res;
}

void * gcmalloc (size_t s, void * meta, size_t count = 1) {
	myfile << "gcmalloc: " << endl;
	size_t size = align_object_size(s * count + sizeof(Object));
	void * res = NULL;
	int pot = power_of_two(size);
	int ss_i = pot - SMALL_BIT_ALIGN - 1;
	myfile << "s = " << s << " meta = " << meta << " count = " << count << " ||| locals: size = " << size << " pot = " << pot << " ss_i = " << ss_i << endl;
	assert(ss_i > -1);
		size_t sle_size = 1 << pot;
		assert(sle_size != 0);
		assert(sle_size == ((size_t)1 << segregated_storage[ss_i].size));
	SegregatedListElement * sle = segregated_storage[ss_i].first != NULL ? segregated_storage[ss_i].first : allocate_new_SegregatedListElement(ss_i);
	assert(sle != NULL);

	// TODO: it is possible to speed up allocation just try only to allocate Obj in sle->descr[i]
	// if success --- cool; otherwise --- allocate new page
	// if descrs are full --> call GC at least in this section
	assert(sle->last_descr <= SSE_DESCR_COUNT);
	for (int i = 0; i < sle->last_descr; i++) {
		PageDescriptor * d = &(sle->descrs[i]);
		if (d->obj_size == 0) { continue; }
		size_t page_end = (size_t)d->page + d->page_size;
		assert((d->free == NULL) || (((size_t)d->free >= (size_t)d->page)) && ((size_t)d->free < page_end));
		if (d->free != NULL) {
			assert(page_end >= (size_t)d->free + d->obj_size);
			void * res = d->free;
			size_t new_free = (size_t)d->free + d->obj_size;
			assert(new_free - (size_t)d->free == d->obj_size);
			d->free =  new_free + d->obj_size <= page_end ? (void *)new_free : NULL;
			myfile << "\t\t res = "<< res << " free = "<< d->free << " page =" << d->page  << " obj_size = " << d->obj_size << " page_size =" << d->page_size << "  mask = " << d->mask << endl;

			Object * obj = (Object *)((size_t)res + size - sizeof(Object));
			myfile << "\t\t obj = " << obj << endl;
			obj->array_count = count;
			obj->meta = meta;
			obj->begin = res;

			assert((void *)((size_t)d->mask & (size_t)((size_t)res + d->obj_size / 2)) == res); /// this asssert checks mask
			myfile << "gcmalloc: end1" << endl;
			return res;
		}
	}

	PageDescriptor * d = NULL;
	if (sle->last_descr < SSE_DESCR_COUNT) {
		d = &(sle->descrs[sle->last_descr]);
		sle->last_descr++;
	} else {
		// TODO: try call GC
		// if enought space after --- allocate
		// otherwise --- allocate new sle
		SegregatedListElement * new_sle = allocate_new_SegregatedListElement(ss_i);
		d = &(sle->descrs[0]);
	}
	assert(d != NULL);
	return allocate_on_new_page(d, size, meta, count);
}

/* returns object header by the pointer somewhere in */
Object * get_object_header (PageDescriptor * d, void * ptr) {
	size_t obj_beg = ((size_t)ptr & d->mask);
	myfile << "get_object_header : obj_beg = " << (void *)obj_beg << endl;
	assert(obj_beg >= (size_t)d->page);
	assert(obj_beg < (size_t)d->free);
	Object * res = (Object *)(obj_beg + d->obj_size - sizeof(Object));
	assert((size_t)res > (size_t)d->page && (size_t)res < (size_t)d->free);
	return res;
}
inline Object * get_object_header (void * ptr) {
	return get_object_header((PageDescriptor *)IT_get_page_descr(ptr), ptr);
}

/* removes one object from page */
void remove_object (void * ptr) {
	PageDescriptor * d = (PageDescriptor *)IT_get_page_descr(ptr);
	myfile << "remove_object : PageDescriptor = " << (void *)d << endl;
	if (d == NULL) { myfile << "remove_object : incorrect pointer" << endl; return; }
	Object * obj = get_object_header(d, ptr);
	myfile << "remove_object : obj = " << (void *)obj << endl;

	if ((size_t)obj + sizeof(Object) == (size_t)d->free) {
		d->free = obj->begin;
		assert((size_t)d->free >= (size_t)d->page);
	}
	myfile << "remove_object : remove " << obj->begin << endl;
	memset(obj->begin, 0, ((size_t)d->obj_size) / sizeof(int));
}

//////////
// MARK //
//////////

PageDescriptor * calculate_ob_i_and_mask (size_t * ob_i, long * mask, void * ptr) {
	PageDescriptor * d = (PageDescriptor *)IT_get_page_descr(ptr);
	if (d == NULL) { return NULL; }
	size_t pot = power_of_two(d->obj_size), bi = SYSTEM_BIT_SIZE - OBJECTS_PER_PAGE_BITS - pot;
	*ob_i = (d->mask << bi) >> (bi + pot);
	size_t mb_i = *ob_i / BITS_PER_LONG,
		bit_i = *ob_i % BITS_PER_LONG;
	*mask = (long)1 << (BITS_PER_LONG - bit_i - 1);
	return d;
}

/* 
mark_pin --- flase => pin bit; true => mark_bit
return values:
	 0 --- succesfully marked
	 1 --- already marked
	-1 --- incorrect pointer(not in the heap) */
int mark_object (void * ptr, bool mark_pin) {
	// ob_i --- calculate object number on the page
	size_t ob_i; long mask; PageDescriptor * d = calculate_ob_i_and_mask(&ob_i, &mask, ptr);
	if (d == NULL) { return -1; }
	long * array = mark_pin == true ? d->mark_bits : d->pin_bits;
	if (array[ob_i] & mask == mask) { return 1; }
	array[ob_i] |= mask;
	return 0;
}

int clear_object_bit (void * ptr, bool mark_pin) {
	size_t ob_i; long mask; PageDescriptor * d = calculate_ob_i_and_mask(&ob_i, &mask, ptr);
	if (d == NULL) { return -1; }
	long * array = mark_pin == true ? d->mark_bits : d->pin_bits;
	array[ob_i] &= ~mask;
	return 0;
}

int clear_all_object_bits (void * ptr) {
	size_t ob_i; long mask; PageDescriptor * d = calculate_ob_i_and_mask(&ob_i, &mask, ptr);
	if (d == NULL) { return -1; }
	d->mark_bits[ob_i] &= ~mask; d->pin_bits[ob_i] &= ~mask;
	return 0;
}

int get_object_bit (void * ptr, bool mark_pin) {
	size_t ob_i; long mask; PageDescriptor * d = calculate_ob_i_and_mask(&ob_i, &mask, ptr);
	if (d == NULL) { return -1; }
	long * array = mark_pin == true ? d->mark_bits : d->pin_bits;
	return (array[ob_i] & mask) == mask;
}

void clear_page_flags (PageDescriptor * d) {
	memset(d->mark_bits, 0, MARK_BITS_ARRAY_SIZE);
	memset(d->pin_bits, 0, MARK_BITS_ARRAY_SIZE);
}

///////////
// TESTS //
///////////
/* look in other files */