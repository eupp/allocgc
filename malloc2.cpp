#include <malloc.h>
#include <assert.h>
#include <unistd.h>
#include "malloc.h"
#include <stdlib.h>
#include <vector>
#include <sys/mman.h>
#include <limits.h>
#include <fstream>
#include <string.h>
#include "go.h"

using namespace std;

ofstream myfile;

#define VIRTUAL_MEMORY_CELL (size_t)sysconf(_SC_PAGE_SIZE)
#define VIRTUAL_MEMORY_CELL_BITS power_of_two(VIRTUAL_MEMORY_CELL)
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
    return (int)(size >> VIRTUAL_MEMORY_CELL_BITS);
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
    size_t i = size & (size -1);
    // if size === power of two then return it
    if (i == 0) { return size; }
    // otherwise --- round it up
    while (i != 0) { size = i; i = size & (size -1); }
    return size << 1;
}
/* align size to be a power of two and to be a multiple of VIRTUAL_MEMORY_CELL */
size_t align_object_size (size_t size) {
    assert(size > 0);
    size_t i = size & (size -1);
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
	//	first to fields together === base_meta
    void * meta; // pointer on the meta information
	size_t count;
	void * begin; // pointer on th object begin
	// NB: FALSE in current version: we use last two bits of begin pointer to be a pin and mask bits respectively
};

// segregated storage page header
/// its size might be aligned on power of two
/* 2^OBJECTS_PER_PAGE_BITS equals to the count of objects on page (by default; can de changed) */
#define OBJECTS_PER_PAGE_BITS	((size_t)5)
#define OBJECTS_PER_PAGE_COUNT	(1 << OBJECTS_PER_PAGE_BITS)
#define BITS_PER_LONG			(sizeof(long) * CHAR_BIT)
#define MARK_BITS_ARRAY_SIZE	(OBJECTS_PER_PAGE_COUNT / BITS_PER_LONG + 1)
struct PageDescriptor {
    size_t obj_size; // sizeof object (whole object (ex. in array of case --- whole array) with a header)
    size_t page_size;
    size_t mask; // a mask for pointers that points on this page (is used to find object begin)
    void * free; // pointer on the next after the last allocated Object. If Page is full --- NULL
    void * page; // pointer on the page itself
    long mark_bits[MARK_BITS_ARRAY_SIZE]; //mark bits for objects in
    long pin_bits[MARK_BITS_ARRAY_SIZE]; //pin bits for objects in
};

////////////////
// INDEX TREE //
////////////////

#define IT_LEVEL_COUNT			((size_t) 3)
#define IT_BITS_IN_USE			(SYSTEM_BIT_SIZE - VIRTUAL_MEMORY_CELL_BITS)
#define IT_FIRST_LEVEL_SIZE		(IT_BITS_IN_USE - (size_t)(IT_BITS_IN_USE / IT_LEVEL_COUNT) * (IT_LEVEL_COUNT - 1))
#define IT_OTHER_LEVELS_SIZE	((size_t)(IT_BITS_IN_USE / IT_LEVEL_COUNT))
size_t * tree_level_one = (size_t *)mmap(NULL, (((size_t) 1) << IT_FIRST_LEVEL_SIZE) * sizeof(size_t),
                                         PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);

void IT_index_new_cell (void * cell, PageDescriptor * page_decr) {
	size_t bits_curr = SYSTEM_BIT_SIZE - IT_FIRST_LEVEL_SIZE;
	size_t i_l = (size_t)cell >> bits_curr;
	size_t * level = tree_level_one;

	for (int i = 0; i < (int)IT_LEVEL_COUNT - 1; i++) {
        level[i_l] = (void *)level[i_l] != NULL ? level[i_l] : (size_t)mmap(NULL, (((size_t) 1) << IT_OTHER_LEVELS_SIZE) * sizeof(size_t),
                                                                            PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
		assert((void *)level[i_l] != NULL);
		level = reinterpret_cast <size_t *> (level[i_l]);
		bits_curr -= IT_OTHER_LEVELS_SIZE;
		i_l = ((size_t)cell >> (bits_curr)) & (((size_t)1 << (IT_OTHER_LEVELS_SIZE)) - 1);
	}
	assert(level[i_l] == 0);
	assert((size_t)cell - (size_t)page_decr->page < page_decr->page_size);
	level[i_l] = (size_t)page_decr;
}

size_t * IT_iterate (void * ptr, size_t * bits_curr, size_t * i_l, size_t * level) {
	for (int i = 0; i < (int)IT_LEVEL_COUNT - 1; i++) {
		if ((void *)level[*i_l] == NULL) { return NULL; }
		level = reinterpret_cast <size_t *> (level[*i_l]);
		*bits_curr -= IT_OTHER_LEVELS_SIZE;
		*i_l = ((size_t)ptr >> (*bits_curr)) & (((size_t)1 << (IT_OTHER_LEVELS_SIZE)) - 1);
	}
	return level;
}

/* return a page ptr points to descriptor */
void * IT_get_page_descr (void * ptr) {
	size_t bits_curr = SYSTEM_BIT_SIZE - IT_FIRST_LEVEL_SIZE;
	size_t i_l = (size_t)ptr >> bits_curr;
	size_t * level = IT_iterate(ptr, &bits_curr, &i_l, tree_level_one);
	return level != NULL ? (void *) level[i_l] : NULL;
}
bool is_heap_pointer(void * ptr) { return IT_get_page_descr(ptr) != NULL; }

/* remove index */
void IT_remove_index (void * cell) {
	size_t bits_curr = SYSTEM_BIT_SIZE - VIRTUAL_MEMORY_CELL_BITS - IT_FIRST_LEVEL_SIZE;
	size_t i_l = (size_t)cell >> bits_curr;
	size_t * level = IT_iterate(cell, &bits_curr, &i_l, tree_level_one);
	if (level == NULL) { myfile << "IT_remove_index : doesn't contain cell " << cell << endl; return; }
	level[i_l] = 0;
}

void IT_print_tree (void) {
	printf("tree:\n");
	size_t * level = tree_level_one, *level2, *level3;
	for (size_t i = 0; i < (((size_t) 1) << IT_OTHER_LEVELS_SIZE); i++) {
		if (level[i] == 0) { continue; }
		level2 = reinterpret_cast<size_t *> ((void *) level[i]);
		for (size_t j = 0; j < (((size_t) 1) << IT_OTHER_LEVELS_SIZE); j++) {
			if (level2[j] == 0) { continue; }
			level3 = reinterpret_cast<size_t *> ((void *) level2[j]);
			for (size_t k = 0; k < (((size_t) 1) << IT_OTHER_LEVELS_SIZE); k++) {
				if (level3[k] == 0) { continue; }
				printf("%zu ", level3[k]);
			} printf("\n\n");
		} printf("\n\n\n");
	}
}

////////////////////
// END INDEX TREE //
////////////////////

// divided on object size
#define SSE_SIZE 4096
#define SSE_DESCR_COUNT ((SSE_SIZE - sizeof(SegregatedListElement)) / sizeof(PageDescriptor))
struct SegregatedListElement {
    SegregatedListElement * next, * prev;
    uint last_descr; // last descr of last allocated page
    PageDescriptor descrs[0];
};
struct SegregatedList {
    uint size; // log_2 (sizeof objects that are stored in this Segregated List)
    SegregatedListElement * first, * last; // pointer on the first SegregatedListElement
};
SegregatedList segregated_storage[SEGREGATED_STORAGE_SIZE];

void init_segregated_storage (void) {
    myfile.open("log.out");

    size_t sle_size = 4; // i.e. min size == 32 (i.e. round_up_to_power_of_two(16(i.e. 16 === sizeof(Object)) + ?))
    for (int i = 0; i < SEGREGATED_STORAGE_SIZE; i++, sle_size++) {
        segregated_storage[i].size = (uint)sle_size;
        segregated_storage[i].first = segregated_storage[i].last = NULL;
    }
}

SegregatedListElement * allocate_new_SegregatedListElement (int ss_i) {
    SegregatedListElement * sle = (SegregatedListElement *)mmap(NULL, SSE_SIZE,
                                                                PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
    sle->next = sle->prev = NULL;
    sle->last_descr = 0;
    for (int i = 0; i < SSE_DESCR_COUNT; i++) { sle->descrs[i] = {0, 0, 0, NULL, NULL}; }
    return sle;
}

size_t calculate_mask (size_t page_size, size_t obj_size, void * page_addr) {
    int page_count_bits = power_of_two(page_size), obj_size_bits = power_of_two(obj_size),
        bit_diff = page_count_bits - obj_size_bits;
    assert(page_count_bits != -1 && obj_size_bits != -1);
    return ((size_t)page_addr | ((1 << bit_diff) - 1) << obj_size_bits);
}

void index_new_page (PageDescriptor * d) {
    size_t page = (size_t)d->page, size = d->page_size;
    for (void * i = (void *)page; (size_t)i - page < size; i = (void *)((size_t)i + VIRTUAL_MEMORY_CELL)) {
	    assert(((size_t)i & ((size_t)1 << VIRTUAL_MEMORY_CELL_BITS) - 1) == 0);
        IT_index_new_cell(i, d);
    }
}

void * allocate_on_new_page (PageDescriptor * d, size_t obj_size, void * meta, size_t count) {
    // i.e. it is no space (following our allocating stotegy)
    // so allocate new page
	assert(d != NULL);
    myfile << "\tallocate_on_new_page" << endl;
    size_t objects_on_page_bits = OBJECTS_PER_PAGE_BITS;
    assert(power_of_two(obj_size) + objects_on_page_bits < 64 - VIRTUAL_MEMORY_CELL_BITS);
    size_t page_size = obj_size << objects_on_page_bits;
    void * new_page = NULL;
    while (new_page == NULL) {
        assert(objects_on_page_bits < 16); // can fails there
        assert(page_size == (obj_size << objects_on_page_bits));
        // NB! if insted of memalign(page_size, page_size) call memalign(VIRTUAL_MEMORY_CELL, page_size);  WILL BE BUG!!!!
        new_page = memalign(page_size, page_size);
        if (new_page == NULL) {
            page_size = (page_size << 1); objects_on_page_bits++;
        } else {
            myfile << "\tmemalign: begin = " << new_page << " end = " << (void *)((size_t)new_page + page_size) << " page_size = " << page_size << endl;
        }
    }
	assert(((size_t)new_page & (((size_t)1 << power_of_two(page_size)) - 1)) == 0);
	assert(((size_t)new_page & ((1 << (OBJECTS_PER_PAGE_BITS + power_of_two(obj_size))) - 1)) == 0);
    d->page = new_page;
    d->obj_size = obj_size;
    d->page_size = page_size;
    d->free = (void *)((size_t)d->page + d->obj_size);
    d->mask = calculate_mask(page_size, obj_size, new_page);
    myfile << "\t\t res = page = " << d->page << " free = " << d->free << " obj_size = " << d->obj_size
        << " page_size = " << d->page_size << " mask = " << d->mask << endl;
    // add new cells to index tree
    index_new_page(d);

    void * res = new_page;
    Object * obj = (Object *)((size_t)res + obj_size - sizeof(Object));
    obj->count = count;
    obj->meta = meta;
    obj->begin = res;
    myfile << "allocate_on_new_page: end\n" << endl;
	assert(d->free > res);
    return res;
}

extern Object * get_object_header (PageDescriptor * d, void * ptr);
/* sets meta information by pointer to the object begin */
int set_meta_after_gcmalloc (void * ptr, void * clMeta) {
	PageDescriptor * d = (PageDescriptor *)IT_get_page_descr(ptr);
	myfile << "set_meta_after_gcmalloc : PageDescriptor = " << (void *)d << endl;
	if (d == NULL) { myfile << "set_meta_after_gcmalloc : incorrect pointer" << endl; return -1; }
	get_object_header(d, ptr)->meta = clMeta;
	return 0;
}
base_meta * get_meta_inf (void * ptr) {  /*!< get the block with meta_inf*/
	PageDescriptor * d = (PageDescriptor *)IT_get_page_descr(ptr);
//	if (!((size_t)ptr - (size_t)d->page < d->page_size)) {IT_print_tree();}
	assert((size_t)ptr - (size_t)d->page < d->page_size);
	myfile << "get_meta_inf: PageDescriptor = " << (void *)d << endl;
	if (d == NULL) { myfile << "get_meta_inf : incorrect pointer" << endl; return NULL; }
	return (base_meta *)get_object_header(d, ptr);
}

// TODO we need some gc strotegy
void * gcmalloc (size_t s, void * meta, size_t count = 1) {
gcmalloc_begin:
    myfile << "gcmalloc: " << endl;
    size_t size = align_object_size(s * count + sizeof(Object));
    void * res = NULL;
    int pot = power_of_two(size), ss_i = pot - (int)SMALL_BIT_ALIGN - 1;
    myfile << "s = " << s << " meta = " << meta << " count = " << count << " ||| locals: size = " << size
        << " pot = " << pot << " ss_i = " << ss_i << endl;
    assert(ss_i > -1);
    size_t sle_size = (size_t)1 << pot;
    assert(sle_size != 0);
    assert(sle_size == ((size_t)1 << segregated_storage[ss_i].size));
	SegregatedListElement * sle = segregated_storage[ss_i].first;
	if (sle == NULL) {
		sle = allocate_new_SegregatedListElement(ss_i);
		segregated_storage[ss_i].first = segregated_storage[ss_i].last = sle;
	}
    assert(sle != NULL);

    // TODO: it is possible to speed up allocation just try only to allocate Obj in sle->descr[i]
    // if success --- cool; otherwise --- allocate new page
    // if descrs are full --> call GC at least in this section
    assert(sle->last_descr <= SSE_DESCR_COUNT);
    for (int i = 0; i < sle->last_descr; i++) {
        PageDescriptor * d = &(sle->descrs[i]);
        if (d->obj_size == 0) { assert(true); }
        size_t page_end = (size_t)d->page + d->page_size;
        assert((d->free == NULL) || (((size_t)d->free >= (size_t)d->page)) && ((size_t)d->free < page_end));
        if (d->free != NULL) {
            assert(page_end >= (size_t)d->free + d->obj_size);
            res = d->free;
            size_t new_free = (size_t)d->free + d->obj_size;
            assert(new_free - (size_t)d->free == d->obj_size);
            d->free = new_free + d->obj_size <= page_end ? (void *)new_free : NULL;
	        assert(d->free > res || d->free == NULL);
            myfile << "\t\t res = "<< res << " free = "<< d->free << " page =" << d->page  << " obj_size = "
                << d->obj_size << " page_size =" << d->page_size << "  mask = " << d->mask << endl;

            Object * obj = (Object *)((size_t)res + size - sizeof(Object));
            myfile << "\t\t obj = " << obj << endl;
            obj->count = count;
            obj->meta = meta;
            obj->begin = res;

	        /// this asssert checks mask
            assert((void *)((size_t)d->mask & (size_t)((size_t)res + d->obj_size / 2)) == res);
            myfile << "gcmalloc: end1" << endl;
            return res;
        }
    }

	assert(sle->last_descr <= SSE_DESCR_COUNT);
    if (sle->last_descr == SSE_DESCR_COUNT) {
        // TODO: after gc call we need to try allocate memory again
	    // TODO: it is suitable to provide full and partial garbage collection strategies together
//	    sle_counter++;
//	    if (sle_counter == 10) {
//		    sle_counter = 0;
//printf("call gc\n");
//		    myfile << "gcmalloc : call gc" << endl;
////		    gc();
//		    myfile << "gcmalloc : after gc goto begin \n" << endl;
//		    goto gcmalloc_begin;
//	    }
        // if enough space after --- allocate
	    // TODO: (look in TODO upstairs)
        // otherwise --- allocate new sle
        SegregatedListElement * new_sle = allocate_new_SegregatedListElement(ss_i);
	    new_sle->next = sle; new_sle->prev = NULL; sle->prev = new_sle; sle = new_sle;
	    segregated_storage[ss_i].first = new_sle;
    }
	sle->last_descr++;
    return allocate_on_new_page(&(sle->descrs[sle->last_descr-1]), size, meta, count);
}

/* returns object header by the pointer somewhere in */
Object * get_object_header (PageDescriptor * d, void * ptr) {
    size_t obj_beg = ((size_t)ptr & d->mask);
    myfile << "get_object_header : obj_beg = " << (void *)obj_beg << endl;
    assert(obj_beg >= (size_t)d->page);
    assert(obj_beg < (size_t)d->free || d->free == NULL);
    Object * res = (Object *)(obj_beg + d->obj_size - sizeof(Object));
    assert((size_t)res > (size_t)d->page && ((size_t)res < (size_t)d->free || d->free == NULL));
	assert((size_t)res - (size_t)ptr < d->obj_size);
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

struct FreeListElement{
	FreeListElement * next;
	void * ptr;
	PageDescriptor * pd;
};

struct FreeList {
	FreeListElement * first, * last;
	FreeList (void) : first(NULL), last(NULL) {}
	void push_end (void * p, PageDescriptor * d) {
		assert(d->page_size!=0);
		assert((size_t)p - (size_t)d->page < d->page_size);
		FreeListElement * new_el = (FreeListElement *)malloc(sizeof(FreeList));
		new_el->next = NULL; new_el->ptr = p; new_el->pd = d;
		assert( (last == NULL && first == NULL) || (last != NULL && first != NULL));
		if (last != NULL) {
			last->next = new_el; last = last->next;
		} else {
			last = first = new_el;
		}
	}
	PageDescriptor * remove_first (size_t * ptr) {
		if (first == NULL) { return NULL; }
		PageDescriptor * res_pd = first->pd;
		assert((size_t)first->ptr - (size_t)res_pd->page < res_pd->page_size);
		*ptr = (size_t)first->ptr;
		FreeListElement * del = first;
		if (first != last) {
			first = first->next;
		} else {
			first = last = NULL;
		}
		free(del);
		return res_pd;
	}

	void clear_list (void) {
		size_t ptr;
		while (this->remove_first(&ptr) != NULL) {}
	}
};
void megre_two_lists_and_free_the_second (FreeList * l1, FreeList *l2) {
	if (l2->last == NULL) {
		assert(l2->first == NULL);
		return;
	}
	if (l1->last == NULL) {
		assert(l1->first == NULL);
		*l1 = *l2;
		assert(l1->first == l2->first && l1->last == l2->last);
		l2->last = l2->first = NULL;
		return;
	}
	assert(l1->first != NULL && l2->first != NULL);
	l1->last->next = l2->first;
	l1->last = l2->last;
	l2->last = l2->first = NULL;
}

/* free memory occupied by sle and removes indexes for all cells in this sle from index tree */
void deallocate_sle (SegregatedListElement * sle) {
	myfile << "deallocate_sle: starts" << endl;
	for (int i = 0; i < SSE_DESCR_COUNT; i++) {
		size_t page = (size_t)sle->descrs[i].page, page_size = sle->descrs[i].page_size;
		for (void * cell = (void *)page; (size_t)cell - page < page_size; cell = (void *)((size_t)cell + VIRTUAL_MEMORY_CELL)) {
			IT_remove_index(cell);
		}
	}
	if (munmap((void *)sle, SSE_SIZE) == -1) {
		myfile << "\tdeallocate_sle: munmap fails!! ==> exit!" << endl;
		assert(true);
	}
	myfile << "\tdeallocate_sle: ends" << endl;
}

// TODO we need some better way to fix pointers! This way is unacceptable
struct MovedObjectsListElement {
	void * from;
	void * to;
	MovedObjectsListElement * next;
};

extern void clear_page_flags(PageDescriptor *);
bool get_next_descr(SegregatedListElement *& sle1, int * descr_i_1, PageDescriptor *& d1, PageDescriptor * d2) {
get_next_descr_begin:
	if (*descr_i_1 < (int)(SSE_DESCR_COUNT - 1)) {
		(*descr_i_1)++;
		if (d1) { clear_page_flags(d1); }
		d1 = &sle1->descrs[*descr_i_1];
		if (d1 == d2) {
			return true;
		}
		if (d1->obj_size == 0) {
			goto get_next_descr_begin;
		} else {
			return false;
		}
	}
	*descr_i_1 = -1;
	sle1 = sle1->next;
	assert (sle1 != NULL);
	goto get_next_descr_begin;
}
bool get_prev_descr(SegregatedListElement *& sle2, int * descr_i_2, PageDescriptor *& d2, PageDescriptor * d1) {
get_prev_descr_begin:
	if (*descr_i_2 > (int)0) {
		(*descr_i_2)--;
		if (d2) { clear_page_flags(d2); }
		d2 = &sle2->descrs[*descr_i_2];
		if (d1 == d2) {
			return true;
		}
		if (d2->obj_size == 0) {
			goto get_prev_descr_begin;
		} else {
			return false;
		}
	}
	*descr_i_2 = SSE_DESCR_COUNT;
	sle2 = sle2->prev;
	assert (sle2 != NULL);
	goto get_prev_descr_begin;
}

struct MovedObjectsList {
	MovedObjectsListElement * first;
	MovedObjectsList () : first(NULL) {}
};
MovedObjectsList mol;
void add_moved_record (void * f, void * t) {
	MovedObjectsListElement * new_rec = (MovedObjectsListElement *)malloc(sizeof(MovedObjectsListElement));
	*new_rec = {f, t, mol.first};
	mol.first = new_rec;
}
void * get_new_destination (void * f) {
	for (MovedObjectsListElement * it = mol.first; it != NULL; it = it->next) {
		if (it->from == f) { return it->to; }
	}
	return NULL;
}

void clear_moved_objects_list (void) {
	for (MovedObjectsListElement * del = mol.first, * prev = NULL; del != NULL; ) {
		prev = del->next;
		free(del);
		del = prev;
	}
	mol.first = NULL;
}
size_t fixed_count = 0, moved_count = 0, marked_objects = 0, marked_objects_in_sweep = 0, pined_objects_in_sweep = 0;

void fix_one_ptr (void * ptr) {
	if (ptr != NULL) {
//		void *next = get_next_obj(ptr);
		void * next = *(void **)ptr;
		if (next != NULL) {
			void * new_destination = get_new_destination(next);
			if (new_destination != NULL) {
//				printf("fix ptr from %p to %p\n", *(void**)ptr, new_destination);
				*(void **) ptr = new_destination; // move_ptr(*(void **) ptr, new_destination);
				fixed_count++;
			}
		}
	}
}
void fix_ptr_in_object(void * object, void * meta_ptr) {
	void * data = object;
	base_meta * meta = (base_meta *)meta_ptr;
	size_t size = meta->shell[0];
	size_t count = meta->shell[1];
	if (count == 0) { return; }
	for (int i = 0; i < meta->count; i++) {
		for (int j = 0; j < count; j++) {
			fix_one_ptr((char *) data + meta->shell[2 + j]);
		}
		data += size;
	}
}

void gcmalloc_fix_ptr (void) {
	int i = 0;
	for (SegregatedList * sl = &segregated_storage[i]; i < SEGREGATED_STORAGE_SIZE; i++, sl = &segregated_storage[i]) {
		for (SegregatedListElement * sle = sl->first; sle != NULL; sle = sle->next) {
			for (uint descr_i = 0; descr_i < SSE_DESCR_COUNT; descr_i++) {
				PageDescriptor *d = &(sle->descrs[descr_i]);
				size_t header_size = sizeof(Object), page_end = (size_t)d->page + d->page_size, object_size = d->obj_size;
				for (Object * obj = (Object *)((size_t)d->page + d->obj_size - header_size); (size_t)obj < page_end;
				                                                        obj = (Object *)((size_t)obj + object_size)) {
					if (obj->meta != NULL) {
						obj->begin = (void *)obj - d->obj_size + sizeof(Object);
						fix_ptr_in_object(obj->begin, (base_meta *) obj);
					}
				}
			}
		}
	}
	// TODO fix_roots in go.cpp
	fix_roots();
	clear_moved_objects_list();
}
extern void clear_page_flags(PageDescriptor *);
void two_fingers_compact_full (void) {
// TODO maybe forward pointers?
// TODO somewhere we need to deallocate free space
	int i = 0;
	for (SegregatedList * sl = &segregated_storage[i]; i < SEGREGATED_STORAGE_SIZE; i++, sl = &segregated_storage[i]) {
		SegregatedListElement * sle1 = sl->first, * sle2 = sl->last;
		if (!sle1) { continue; }
		int descr_i_1 = -1, descr_i_2 = SSE_DESCR_COUNT - 1;
		PageDescriptor * d1 = &sle2->descrs[descr_i_2], * d2 = &sle1->descrs[descr_i_1];
		bool stop = get_next_descr(sle1, &descr_i_1, d1, d2) || get_prev_descr(sle2, &descr_i_2, d2, d1);
		uint bit_in_size_t = BITS_PER_LONG;
		int sf_i = OBJECTS_PER_PAGE_COUNT - 1, ff_i = 0, last_occupated_object = -1;
		bool sf_mark = false, sf_pin, ff_mark = true, ff_pin, descrs_met = false;
		while (true) {
			while (!sf_mark) {
				if (sf_i == -1) {
					if (descrs_met) {
						goto end_action;
					}
					if (d2 != NULL) {
						d2->free = last_occupated_object == -1 || last_occupated_object == OBJECTS_PER_PAGE_COUNT - 1
						           ? NULL
                                   : (void *)((size_t)d2->page + d2->obj_size * (last_occupated_object + 1));
					}
					clear_page_flags(d2);
					stop = stop || get_prev_descr(sle2, &descr_i_2, d2, d1);
					if (stop) {
						descrs_met = true;
					}
					last_occupated_object = -1, sf_mark = false, sf_pin = false, sf_i = OBJECTS_PER_PAGE_COUNT - 1;
				}
				sf_mark = (bool) (d2->mark_bits[sf_i / bit_in_size_t] &
				                  ((long) 1 << (bit_in_size_t - (sf_i % bit_in_size_t) - 1)));
				sf_pin = (bool) (d2->pin_bits[sf_i / bit_in_size_t] &
				                 ((long) 1 << (bit_in_size_t - (sf_i % bit_in_size_t) - 1)));
				if (sf_mark) {
					marked_objects_in_sweep++;
					goto start_loop_ff;
				} else if (sf_pin) {
					pined_objects_in_sweep++;
					last_occupated_object = last_occupated_object == -1 ? sf_i : last_occupated_object;
				} else {
					memset((void *) ((size_t) d2->page + d2->obj_size * sf_i), 0, d2->obj_size);
				}
				sf_i--;
			}

			start_loop_ff:
			while (ff_mark) {
				if (ff_i == OBJECTS_PER_PAGE_COUNT) {
					if (descrs_met) {
						if (d2 != NULL) {
							d2->free = last_occupated_object == -1 || last_occupated_object == OBJECTS_PER_PAGE_COUNT - 1
							           ? NULL
							           : (void *)((size_t)d2->page + d2->obj_size * (last_occupated_object + 1));
						}
						goto end_action;
					}
					if (d1 != NULL) {
						d1->free = NULL;
					}
					clear_page_flags(d1);
					stop = stop || get_next_descr(sle1, &descr_i_1, d1, d2);
					if (stop) {
						descrs_met = true;
					}
					ff_mark = false; ff_pin = false; ff_i = 0;
				}
				ff_mark = (bool)(d1->mark_bits[ff_i / bit_in_size_t] & ((long)1 << (bit_in_size_t - (ff_i % bit_in_size_t) - 1)));
				ff_pin = (bool)(d1->pin_bits[ff_i / bit_in_size_t] & ((long)1 << (bit_in_size_t - (ff_i % bit_in_size_t) - 1)));
				if (ff_mark) {
					marked_objects_in_sweep++;
				} else if (ff_pin) {
					pined_objects_in_sweep++;
				} else {
					memset((void *)((size_t)d1->page + d1->obj_size * ff_i), 0, d1->obj_size);
					goto move_object;
				}
				ff_i++;
			}
			if (ff_i >= sf_i && descrs_met) {
				clear_page_flags(d1);
				goto end_action;
			}

			move_object:
			void * to_place = (void *)((size_t)d1->page + d1->obj_size * ff_i),
					* from_place = (void *)((size_t)d2->page + d2->obj_size * sf_i);
			memcpy(to_place, from_place, d1->obj_size);
			memset(from_place, 0, d1->obj_size);
			// TODO change to forward pointer!
			add_moved_record(from_place, to_place);
//			printf("moved %p to %p %i %i\n", from_place, to_place, sf_i, ff_i);
			moved_count++;

			sf_mark = false, ff_mark = true, ff_i++, sf_i--;
		}
	}
	end_action:
	gcmalloc_fix_ptr();
	printf("moved count == %zu\n", moved_count);
	printf("fixed count == %zu\n", fixed_count);
	printf("marked count == %zu %zu pined count = %zu\n", marked_objects, marked_objects_in_sweep, pined_objects_in_sweep);
	moved_count = fixed_count = marked_objects_in_sweep = marked_objects_in_sweep = pined_objects_in_sweep = 0;
}

void two_fingers_compact (void) {
// TODO rewrite to two fingers algorithm and then maybe forward pointers?
	int i = 0;
	for (SegregatedList * sl = &segregated_storage[i]; i < SEGREGATED_STORAGE_SIZE; i++, sl = &segregated_storage[i]) {
		for (SegregatedListElement * sle = sl->first; sle != NULL; sle = sle->next) {
			for (uint descr_i = 0; descr_i < SSE_DESCR_COUNT; descr_i++) {
				PageDescriptor *d = &(sle->descrs[descr_i]);
				if (d->obj_size == 0) {
					continue;
				}
				int ff_i = -1, sf_i = OBJECTS_PER_PAGE_COUNT + 1;
				uint bit_in_size_t = BITS_PER_LONG;
				bool ff_mark , ff_pin, sf_mark, sf_pin;
				while (true) {
					do {
						sf_i--;
						sf_mark = (bool)(d->mark_bits[sf_i / bit_in_size_t] & ((long)1 << (bit_in_size_t - (sf_i % bit_in_size_t) - 1)));
						sf_pin = (bool)(d->pin_bits[sf_i / bit_in_size_t] & ((long)1 << (bit_in_size_t - (sf_i % bit_in_size_t) - 1)));
						if (sf_mark) {
							marked_objects_in_sweep++;
						} else if (sf_pin) {
							pined_objects_in_sweep++;
						} else {
							memset((void *)((size_t)d->page + d->obj_size * sf_i), 0, d->obj_size);
						}
					} while ((!sf_mark || sf_pin) && ff_i < sf_i);
					do {
						if (ff_i >= sf_i - 1) {
							goto fingers_met;
						}
						ff_i++;
						ff_mark = (bool)(d->mark_bits[ff_i / bit_in_size_t] & ((long)1 << (bit_in_size_t - (ff_i % bit_in_size_t) - 1)));
						ff_pin = (bool)(d->pin_bits[ff_i / bit_in_size_t] & ((long)1 << (bit_in_size_t - (ff_i % bit_in_size_t) - 1)));
						if (ff_mark && ff_i != sf_i) {
							marked_objects_in_sweep++;
						} else if (ff_pin) {
							pined_objects_in_sweep++;
						} else {
							memset((void *)((size_t)d->page + d->obj_size * ff_i), 0, d->obj_size);
						}
					} while ((ff_mark || ff_pin) &&  ff_i < sf_i);
					if (ff_i >= sf_i) {
						goto fingers_met;
					}
					// move object
					void * to_place = (void *)((size_t)d->page + d->obj_size * ff_i),
							* from_place = (void *)((size_t)d->page + d->obj_size * sf_i);
					memcpy(to_place, from_place, d->obj_size);
					memset(from_place, 0, d->obj_size);
					// TODO change to forward pointer!
					add_moved_record(from_place, to_place);
					printf("moved %p to %p %i %i\n", from_place, to_place, sf_i, ff_i);
					moved_count++;
				}
				fingers_met:
				d->free = sf_i != OBJECTS_PER_PAGE_COUNT ? (void *)((size_t)d->page + d->obj_size * (sf_i + 1)) : NULL;
				clear_page_flags(d);
			}
		}
	}
	gcmalloc_fix_ptr();
	printf("moved count == %zu\n", moved_count);
	printf("fixed count == %zu\n", fixed_count);
	printf("marked count == %zu %zu pined count = %zu\n", marked_objects, marked_objects_in_sweep, pined_objects_in_sweep);
	moved_count = fixed_count = marked_objects_in_sweep = marked_objects_in_sweep = pined_objects_in_sweep = 0;
}

/* produces segregated storage compactification
 * if some SegregatedListElement is fully free --- calls deallocates its deallocation
 * clears mark and pin bits
 * calls fix_ptr after compactification
 * */
void sweep (void) {
// TODO check this function
// TODO rewrite to two fingers algorithm and then maybe forward pointers?
	int i = 0;
	for (SegregatedList * sl = &segregated_storage[i]; i < SEGREGATED_STORAGE_SIZE; i++, sl = &segregated_storage[i]) {
		FreeList * sl_freeList = (FreeList *)malloc(sizeof(FreeList));
		sl_freeList->first = sl_freeList->last = NULL;
		assert(sl_freeList->first == NULL && sl_freeList->last == NULL);
		for (SegregatedListElement * sle = sl->first, * sle_prev = NULL; sle != NULL;) {
			FreeList * sle_freeList = (FreeList *)malloc(sizeof(FreeList));
			sle_freeList->first = sle_freeList->last = NULL;
			assert(sle_freeList->first == NULL && sle_freeList->last == NULL);
			uint last_occupied_descr = 0;
			for (uint descr_i = 0; descr_i < SSE_DESCR_COUNT; descr_i++) {
				PageDescriptor *d = &(sle->descrs[descr_i]);
				if (d->obj_size == 0) { continue; }
				assert(d->obj_size != 0);
				void * last_occupied_object_on_page = NULL;
				// iterator throw pin and mark
				size_t qwe = MARK_BITS_ARRAY_SIZE;
				for (size_t mark_pin_i = 0; mark_pin_i < MARK_BITS_ARRAY_SIZE; mark_pin_i++) {
					long mark_el = d->mark_bits[mark_pin_i], pin_el = d->pin_bits[mark_pin_i];
					for (int in_mark_pin_i = CHAR_BIT * sizeof(size_t) - 1; in_mark_pin_i >= 0; in_mark_pin_i--) {
						// TODO this if might be changed
						if (in_mark_pin_i - OBJECTS_PER_PAGE_COUNT < 0) {
							break;
						}
						size_t place_in_i = (size_t) 1 << in_mark_pin_i;
						bool mark = (bool) (mark_el & place_in_i), pin = (bool) (pin_el & place_in_i);
						void * current_object_begin = (void *) ((size_t) d->page +((size_t) mark_pin_i * CHAR_BIT * sizeof(size_t)
						                                                           + CHAR_BIT * sizeof(size_t) - 1 - in_mark_pin_i) * d->obj_size);
						assert((size_t)current_object_begin - (size_t)d->page < d->page_size);
						if (mark) {
							marked_objects_in_sweep++;
							if (pin) { // cannot move object
								last_occupied_object_on_page = current_object_begin;
								continue;
							} else { // try to move object if possible
								size_t to_place = 0;
								PageDescriptor * destination_d = sle_freeList->remove_first(&to_place);
								assert(destination_d == NULL || destination_d->obj_size != (size_t) 0);
								if (to_place == 0) {
									destination_d = sl_freeList->remove_first(&to_place);
									assert(destination_d == NULL || destination_d->obj_size != (size_t) 0);
								}
								assert(destination_d == NULL || to_place != 0);
								if (to_place != 0) {
									// move object
									memcpy((void *)to_place, current_object_begin, d->obj_size);
									void * bm1 = ((base_meta *)((void *)to_place + d->obj_size - sizeof(Object)))->shell,
										* bm2 = ((base_meta *)(current_object_begin + d->obj_size - sizeof(Object)))->shell;
									// clear memory area that was occupied by object was moved
									assert((void *)to_place != current_object_begin);
									assert(destination_d->obj_size == d->obj_size);
									assert(current_object_begin + d->obj_size <= (void*)to_place ||
											       current_object_begin >= (void*)to_place + d->obj_size);
									memset(current_object_begin, 0, d->obj_size);
									assert(bm1 == bm2);
									// if object is moved on current page then update last_occupied_object_on_page
									if ((destination_d->page == d->page) && (size_t)to_place > (size_t)last_occupied_object_on_page) {
										last_occupied_object_on_page = (void *)to_place;
									}
									// update destination_d.free if necessary
//									assert(destination_d->free != NULL);
									if (destination_d->free != NULL) {
										void *new_destination_d_free = (void *) (to_place + destination_d->obj_size);
										assert((size_t) new_destination_d_free <=
										       (size_t) destination_d->page + destination_d->page_size);
										if ((size_t) new_destination_d_free > (size_t) destination_d->free) {
											destination_d->free = ((size_t) new_destination_d_free ==
											                       (size_t) destination_d->page +
											                       destination_d->page_size) ?
											                      NULL : new_destination_d_free;
										}
										assert( destination_d->free == NULL ||
										        ((size_t) destination_d->free > (size_t) destination_d->page
										       && (size_t) destination_d->free <
										          (size_t) destination_d->page + destination_d->page_size));
									}
									// add record about this movement in a table
									add_moved_record(current_object_begin, (void *)to_place);
//									printf("objetc %p moved to %p\n", current_object_begin, (void *)to_place);
									assert(destination_d->free != destination_d->page);
									moved_count++;
								} else { // object is not moved ==> update last_occupied_object_on_page
									last_occupied_object_on_page = current_object_begin;
								}
							}
						} else { // add this object to free list
							sle_freeList->push_end(current_object_begin, d);
						}
					}
					// clear mark and pin bits
					d->mark_bits[mark_pin_i] = (size_t) 0; d->pin_bits[mark_pin_i] = (size_t) 0;
				}
				if (last_occupied_object_on_page == NULL) {
					d->free = d->page;
					memset(d->page, 0, d->page_size);
				} else { // update d->free and last_occupied_descr
					last_occupied_descr = descr_i;
					void * new_d_free = (void *)((size_t)last_occupied_object_on_page + d->obj_size);
					d->free = (size_t)new_d_free < (size_t)d->page + d->page_size ? new_d_free : NULL;
					assert(d->free == NULL || ((size_t)d->free >= (size_t)d->page) && (size_t)d->free < (size_t)d->page + d->page_size);
				}
			}
			if (last_occupied_descr == 0) { // remove this sle
				SegregatedListElement * next_sle = sle->next;
				if (sle_prev == NULL) {
					sl->first = sle->next;
				} else {
					sle_prev->next = sle->next;
				}
				// remove sle freeList
				sle_freeList->clear_list();
				// remove sle itself
				deallocate_sle(sle);
				sle = next_sle;
			} else {
				sle->last_descr = last_occupied_descr + 1;
				assert(sle->last_descr <= SSE_DESCR_COUNT);
				sle_prev = sle;
				// save sle_freeList in sl_freeList
				megre_two_lists_and_free_the_second(sl_freeList, sle_freeList);
				sle = sle->next;
			}
			free(sle_freeList);
		}
		sl_freeList->clear_list();
		free(sl_freeList);
	}
	printf("moved xount == %zu\n", moved_count);
	printf("fixed xount == %zu\n", fixed_count);
	printf("marked xount == %zu %zu\n", marked_objects, marked_objects_in_sweep);
	gcmalloc_fix_ptr();
}

//////////
// MARK //
//////////

PageDescriptor * calculate_ob_i_and_mask (size_t * ob_i, long * mask, void * ptr) {
    PageDescriptor * d = (PageDescriptor *)IT_get_page_descr(ptr);
    if (d == NULL) { return NULL; }
    int pot = power_of_two(d->obj_size), bi = (int)SYSTEM_BIT_SIZE - (int)OBJECTS_PER_PAGE_BITS - pot;
    *ob_i = ((d->mask & (size_t)ptr) << bi) >> (bi + pot);
assert(ptr == d->page + *ob_i * d->obj_size);
    size_t bit_i = *ob_i % BITS_PER_LONG;
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
    long * array = mark_pin ? d->mark_bits : d->pin_bits;
    if ((array[ob_i / BITS_PER_LONG] & mask) == mask) { return 1; }
    array[ob_i / BITS_PER_LONG] |= mask;
	marked_objects++;
    return 0;

}

int clear_object_bit (void * ptr, bool mark_pin) {
    size_t ob_i; long mask; PageDescriptor * d = calculate_ob_i_and_mask(&ob_i, &mask, ptr);
    if (d == NULL) { return -1; }
    long * array = mark_pin ? d->mark_bits : d->pin_bits;
    array[ob_i / BITS_PER_LONG] &= ~mask;
    return 0;
}

int clear_all_object_bits (void * ptr) {
    size_t ob_i; long mask; PageDescriptor * d = calculate_ob_i_and_mask(&ob_i, &mask, ptr);
    if (d == NULL) { return -1; }
    d->mark_bits[ob_i / BITS_PER_LONG] &= ~mask; d->pin_bits[ob_i / BITS_PER_LONG] &= ~mask;
    return 0;
}

int get_object_mark (void * ptr, bool mark_pin) {
    size_t ob_i; long mask; PageDescriptor * d = calculate_ob_i_and_mask(&ob_i, &mask, ptr);
    if (d == NULL) { return -1; }
    long * array = mark_pin ? d->mark_bits : d->pin_bits;
    return (array[ob_i / BITS_PER_LONG] & mask) == mask;
}

void clear_page_flags (PageDescriptor * d) {
	if (d == NULL) { return; }
    memset(d->mark_bits, 0, MARK_BITS_ARRAY_SIZE * sizeof(long));
    memset(d->pin_bits, 0, MARK_BITS_ARRAY_SIZE * sizeof(long));
}

bool mark_after_overflow() {
// TODO write it
	myfile << "mark_after_overflow is called! ERROR! Exit!" << endl;
	assert(true);
	return true;
}

///////////
// TESTS //
///////////
/* look in other files */