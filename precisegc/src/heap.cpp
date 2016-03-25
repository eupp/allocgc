#include "heap.h"

#include "go.h"
#include "index_tree.h"
#include "gc_ptr.h"

//////////////
// GCMALLOC //
//////////////

using namespace _GC_;

static pthread_mutex_t malloc_mutexes[SegregatedStorageSize];
static const int init_res = init_segregated_storage();

void lock_all_mutexes (void) {
	for (int i = 0; i < SegregatedStorageSize; i++) {
		pthread_mutex_lock(&malloc_mutexes[i]);
	}
}
void unlock_all_mutexes (void) {
	for (int i = 0; i < SegregatedStorageSize; i++) {
		pthread_mutex_unlock(&malloc_mutexes[i]);
	}
}

bool _GC_::is_heap_pointer(void *ptr) { return IT_get_page_descr(ptr) != NULL; }

int _GC_::init_segregated_storage (void) {
//    myfile.open("log.out");

    size_t sle_size = 4; // i.e. min size == 32 (i.e. round_up_to_power_of_two(16(i.e. 16 === sizeof(Object)) + ?))
    for (int i = 0; i < SegregatedStorageSize; i++, sle_size++) {
        segregated_storage[i].size = (uint)sle_size;
        segregated_storage[i].first = segregated_storage[i].last = NULL;
	    malloc_mutexes[i] = PTHREAD_MUTEX_INITIALIZER;
    }
	return 0;
}

SegregatedListElement * allocate_new_SegregatedListElement (void) {
    SegregatedListElement * sle = (SegregatedListElement *)mmap(NULL, SSESize,
                                   PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
    sle->next = sle->prev = NULL;
    sle->last_descr = 0;
    for (int i = 0; i < SSEDescrCount; i++) { sle->descrs[i] = {0, 0, 0, NULL, NULL}; }
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
    for (void * i = (void *)page; (size_t)i - page < size; i = (void *)((size_t)i + MemoryCell)) {
	    assert(((size_t)i & ((size_t)1 << CellBits) - 1) == 0);
        IT_index_new_cell(i, d);
    }
}

void * allocate_on_new_page (PageDescriptor * d, size_t obj_size, void * meta, size_t count) {
    // i.e. it is no space (following our allocating stotegy)
    // so allocate new page
	assert(d != NULL);
//    myfile << "\tallocate_on_new_page" << endl;
    size_t objects_on_page_bits = ObjectsPerPageBits;
    assert(power_of_two(obj_size) + objects_on_page_bits < 64 - CellBits);
    size_t page_size = obj_size << objects_on_page_bits;
    void * new_page = NULL;
    while (new_page == NULL) {
        assert(objects_on_page_bits < 16); // can fails there
        assert(page_size == (obj_size << objects_on_page_bits));
        // NB! if insted of memalign(page_size, page_size) call memalign(MemoryCell, page_size);  WILL BE BUG!!!!
        new_page = memalign(page_size, page_size);
        if (new_page == NULL) {
            page_size = (page_size << 1); objects_on_page_bits++;
        } else {
//            myfile << "\tmemalign: begin = " << new_page << " end = " << (void *)((size_t)new_page + page_size) << " page_size = " << page_size << endl;
        }
    }
	assert(((size_t)new_page & (((size_t)1 << power_of_two(page_size)) - 1)) == 0);
	assert(((size_t)new_page & ((1 << (OBJECTS_PER_PAGE_BITS + power_of_two(obj_size))) - 1)) == 0);
    d->page = new_page;
    d->obj_size = obj_size;
    d->page_size = page_size;
    d->free = (void *)((size_t)d->page + d->obj_size);
    d->mask = calculate_mask(page_size, obj_size, new_page);
//    myfile << "\t\t res = page = " << d->page << " free = " << d->free << " obj_size = " << d->obj_size
//        << " page_size = " << d->page_size << " mask = " << d->mask << endl;
    // add new cells to index tree
    index_new_page(d);

    void * res = new_page;
    Object * obj = (Object *)((size_t)res + obj_size - sizeof(Object));
    obj->count = count;
    obj->meta = meta;
    obj->begin = res;
//    myfile << "allocate_on_new_page: end\n" << endl;
	assert(d->free > res);
    return res;
}

extern Object * get_object_header (PageDescriptor * d, void * ptr);
/* sets meta information by pointer to the object begin */
int _GC_::set_meta_after_gcmalloc (void * ptr, void * clMeta) {
	PageDescriptor * d = (PageDescriptor *)IT_get_page_descr(ptr);
//	myfile << "set_meta_after_gcmalloc : PageDescriptor = " << (void *)d << endl;
	if (d == NULL) { /*myfile << "set_meta_after_gcmalloc : incorrect pointer" << endl;*/ return -1; }
	get_object_header(d, ptr)->meta = clMeta;
	return 0;
}
pthread_mutex_t get_meta_inf_mutex = PTHREAD_MUTEX_INITIALIZER;
base_meta * _GC_::get_meta_inf (void * ptr) {  /*!< get the block with meta_inf*/
	pthread_mutex_lock(&get_meta_inf_mutex);
	PageDescriptor * d = (PageDescriptor *)IT_get_page_descr(ptr);
//	if (!((size_t)ptr - (size_t)d->page < d->page_size)) {IT_print_tree();}
	assert((size_t)ptr - (size_t)d->page < d->page_size);
	if (d->free != NULL && ! ((size_t)ptr < (size_t)d->free)) {
		assert(d->free == NULL || (size_t) ptr < (size_t) d->free);
	}
//	myfile << "get_meta_inf: PageDescriptor = " << (void *)d << endl;
	if (d == NULL) { /*myfile << "get_meta_inf : incorrect pointer" << endl;*/ return NULL; }
//	return (base_meta *)get_object_header(d, ptr);

	base_meta * res = (base_meta *)get_object_header(d, ptr);
	pthread_mutex_unlock(&get_meta_inf_mutex);
	return res;
}

// TODO we need some gc strategy
void * _GC_::gcmalloc (size_t s, void * meta, size_t count = 1) {
gcmalloc_begin:
//    myfile << "gcmalloc: " << endl;
    size_t size = align_object_size(s * count + sizeof(Object));
    void * res = NULL;
    int pot = power_of_two(size), ss_i = pot - (int) SmallBitAlign - 1;
//    myfile << "s = " << s << " meta = " << meta << " count = " << count << " ||| locals: size = " << size
//        << " pot = " << pot << " ss_i = " << ss_i << endl;
    assert(ss_i > -1);
    size_t sle_size = (size_t)1 << pot;
    assert(sle_size == ((size_t)1 << segregated_storage[ss_i].size));

	pthread_mutex_lock(&malloc_mutexes[ss_i]);
//	lock_all_mutexes();

	SegregatedListElement * sle = segregated_storage[ss_i].first;
	if (sle == NULL) {
		sle = allocate_new_SegregatedListElement();
		segregated_storage[ss_i].first = segregated_storage[ss_i].last = sle;
	}

    // TODO: it is possible to speed up allocation just try only to allocate Obj in sle->descr[i]
    // if success --- cool; otherwise --- allocate new page
    // if descrs are full --> call GC at least in this section
    assert(sle->last_descr <= SSEDescrCount);
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
//            myfile << "\t\t res = "<< res << " free = "<< d->free << " page =" << d->page  << " obj_size = "
//                << d->obj_size << " page_size =" << d->page_size << "  mask = " << d->mask << endl;

            Object * obj = (Object *)((size_t)res + size - sizeof(Object));
            *obj = {meta, count, res};

	        /// this assert checks mask
            assert((void *)((size_t)d->mask & (size_t)((size_t)res + d->obj_size / 2)) == res);
//            myfile << "gcmalloc: end1" << endl;
	        pthread_mutex_unlock(&malloc_mutexes[ss_i]);
//	        unlock_all_mutexes();
            return res;
        };
    }

	assert(sle->last_descr <= SSEDescrCount);
    if (sle->last_descr == SSEDescrCount) {
        // TODO: after gc call we need to try allocate memory again
	    // TODO: it is suitable to provide full and partial garbage collection strategies together
//		    myfile << "gcmalloc : call gc" << endl;
//		    gc();
//		    myfile << "gcmalloc : after gc goto begin \n" << endl;
//		    goto gcmalloc_begin;
        // if enough space after --- allocate
	    // TODO: (look in TODO upstairs)
        // otherwise --- allocate new sle
        SegregatedListElement * new_sle = allocate_new_SegregatedListElement();
	    new_sle->next = sle; new_sle->prev = NULL; sle->prev = new_sle; sle = new_sle;
	    segregated_storage[ss_i].first = new_sle;
    }
	sle->last_descr++;
	res = allocate_on_new_page(&(sle->descrs[sle->last_descr-1]), size, meta, count);
	pthread_mutex_unlock(&malloc_mutexes[ss_i]);
//	unlock_all_mutexes();
    return res;
}

/* returns object header by the pointer somewhere in */
Object * get_object_header (PageDescriptor * d, void * ptr) {
    size_t obj_beg = ((size_t)ptr & d->mask);
//    myfile << "get_object_header : obj_beg = " << (void *)obj_beg << endl;
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
void _GC_::remove_object (void * ptr) {
    PageDescriptor * d = (PageDescriptor *)IT_get_page_descr(ptr);
//    myfile << "remove_object : PageDescriptor = " << (void *)d << endl;
    if (d == NULL) { /*myfile << "remove_object : incorrect pointer" << endl;*/ return; }
    Object * obj = get_object_header(d, ptr);
//    myfile << "remove_object : obj = " << (void *)obj << endl;

    if ((size_t)obj + sizeof(Object) == (size_t)d->free) {
        d->free = obj->begin;
        assert((size_t)d->free >= (size_t)d->page);
    }
//    myfile << "remove_object : remove " << obj->begin << endl;
    memset(obj->begin, 0, ((size_t)d->obj_size) / sizeof(int));
}

/* free memory occupied by sle and removes indexes for all cells in this sle from index tree */
void deallocate_sle (SegregatedListElement * sle) {
//	myfile << "deallocate_sle: starts" << endl;
	for (int i = 0; i < SSEDescrCount; i++) {
		size_t page = (size_t)sle->descrs[i].page, page_size = sle->descrs[i].page_size;
		for (void * cell = (void *)page; (size_t)cell - page < page_size; cell = (void *)((size_t)cell + MemoryCell)) {
			IT_remove_index(cell);
		}
	}
	if (munmap((void *)sle, SSESize) == -1) {
//		myfile << "\tdeallocate_sle: munmap fails!! ==> exit!" << endl;
		assert(true);
	}
//	myfile << "\tdeallocate_sle: ends" << endl;
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
	if (*descr_i_1 < (int)(SSEDescrCount - 1)) {
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
	*descr_i_2 = SSEDescrCount;
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
void * _GC_::get_new_destination (void * f) {
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

#define forward_pointer(ptr)            (*((void**) ptr))
#define set_forward_pointer(from, to)   (*((void**)from) = to)

void _GC_::fix_one_ptr (void * ptr) {
	if (ptr != NULL) {
		void * next = *(void **)ptr;
//		void * next = get_next_obj(ptr);
//		void * next = forward_pointer(ptr);
		if (next != NULL) {
//			void * new_destination = get_new_destination(next);
			if (get_new_destination(next) != NULL) {
				void *new_destination = forward_pointer(next);
				if (new_destination != NULL) {
				printf("fix ptr from %p to %p\n", *(void**)ptr, new_destination);
					*(void **) ptr = new_destination;
//				*(void **) ptr = move_ptr(*(void **) ptr, new_destination);
					fixed_count++;
				}
			}
		}
	}
}

void fix_ptr_in_object(void * object, void * meta_ptr) {
	// TODO add composite and stack flags??
	void * data = object;
	base_meta * meta = (base_meta *)meta_ptr;
	size_t size = meta->shell[0];
	size_t count = meta->shell[1];
	if (count == 0) { return; }
	for (int i = 0; i < meta->count; i++) {
		for (int j = 0; j < count; j++) {
			fix_one_ptr((char *) data + meta->shell[2 + j]);
		}
		data = (void *)((size_t)data + size);
	}
}

void gcmalloc_fix_ptr (void) {
	int i = 0;
	for (SegregatedList * sl = &segregated_storage[i]; i < SegregatedStorageSize; i++, sl = &segregated_storage[i]) {
		for (SegregatedListElement * sle = sl->first; sle != NULL; sle = sle->next) {
			for (uint descr_i = 0; descr_i < SSEDescrCount; descr_i++) {
				PageDescriptor *d = &(sle->descrs[descr_i]);
				size_t header_size = sizeof(Object), page_end = (size_t)d->page + d->page_size, object_size = d->obj_size;
				for (Object * obj = (Object *)((size_t)d->page + d->obj_size - header_size); (size_t)obj < page_end;
				                                                        obj = (Object *)((size_t)obj + object_size)) {
					if (obj->meta != NULL) {
						obj->begin = (void *)((size_t)obj - d->obj_size + sizeof(Object));
						fix_ptr_in_object(obj->begin, (base_meta *) obj);
					}
				}
			}
		}
	}
	fix_roots();
	clear_moved_objects_list();
}

extern void clear_page_flags(PageDescriptor *);
void _GC_::two_fingers_compact_full (void) {
// TODO maybe forward pointers? --- how to know where is forward pointer?
// TODO somewhere we need to deallocate free space
	int i = 0;
	lock_all_mutexes();
	for (SegregatedList * sl = &segregated_storage[i]; i < SegregatedStorageSize; i++, sl = &segregated_storage[i]) {
		// iterate throw segregated list elements and compact memory in each sle
		// sle1 --- iterate from the begin to venue (first finger <=> ff_)
		// sle2 --- iterate from the end to venue (last finger <=> sf_)
		SegregatedListElement * sle1 = sl->first, * sle2 = sl->last;
		if (!sle1) { continue; }
		int descr_i_1 = -1, descr_i_2 = SSEDescrCount;
		PageDescriptor * d1 = NULL, * d2 = NULL;
		bool descrs_met = get_next_descr(sle1, &descr_i_1, d1, d2) || get_prev_descr(sle2, &descr_i_2, d2, d1);

//		unlock_all_mutexes();
//		return;

		uint bit_in_size_t = BitsPerLong;
		// ff_i -- (first finger) descriptor index in the current first finger sle (sle1)
		// sf_i -- (second finger) descriptor index in the current second finger sle (sle2)
		int sf_i = ObjectsPerPage - 1, ff_i = 0, last_occupied_object = -1;
//		bool sf_mark = false, sf_pin, ff_mark = true, ff_pin;
		int sf_mark = false, sf_pin, ff_mark = true, ff_pin;
		void * to_place = NULL;
		while (true) {
			// this loop finds alive object to be moved (second finger)
			while (!sf_mark) {
				if (sf_i == -1) {
					clear_page_flags(d2);
					int last_alive_object_on_page = descrs_met ? max(last_occupied_object, ff_i) : last_occupied_object;
					assert(d2 != NULL);
//					d2->free = ObjectsPerPage - last_alive_object_on_page < 2
//					           ? NULL
//					           : (void *)((size_t)d2->page + d2->obj_size * (last_alive_object_on_page + 1));
					if (descrs_met) {
						goto descrs_met_flag;
					}
					descrs_met = descrs_met || get_prev_descr(sle2, &descr_i_2, d2, d1);
					last_occupied_object = -1, sf_mark = false, sf_pin = false, sf_i = ObjectsPerPage - 1;
				}
//				sf_mark = (bool) (d2->mark_bits[sf_i / bit_in_size_t] &
//				                  ((long) 1 << (bit_in_size_t - (sf_i % bit_in_size_t) - 1)));
//				sf_pin = (bool) (d2->pin_bits[sf_i / bit_in_size_t] &
//				                 ((long) 1 << (bit_in_size_t - (sf_i % bit_in_size_t) - 1)));
				sf_mark = get_object_mark((void *)((size_t)d2->page + d2->obj_size * sf_i), true);
				sf_pin = get_object_mark((void *)((size_t)d2->page + d2->obj_size * sf_i), false);
				assert(sf_mark != -1 && sf_pin != -1);
				if (descrs_met && ff_i >= sf_i) {
					assert(d1 == d2);
					clear_page_flags(d2);
//					d2->free = (void *)((size_t)d2->page + d2->obj_size * (max(sf_i + 1, last_occupied_object + 1)));
					goto descrs_met_flag;
				}
//				if (sf_mark) {
				if (sf_mark == 1) {
 					marked_objects_in_sweep++;
					goto start_loop_ff;
//				} else if (sf_pin) {
				} else if (sf_pin == 1) {
					pined_objects_in_sweep++;
					last_occupied_object = last_occupied_object == -1 ? sf_i : last_occupied_object;
				} else {
					memset((void *) ((size_t) d2->page + d2->obj_size * sf_i), 0, d2->obj_size);
				}
				sf_i--;
			}

			// this loop finds free place where object alive founded in previous loop will be moved (first finger)
			start_loop_ff:
			while (ff_mark) {
				if (ff_i == ObjectsPerPage) {
					clear_page_flags(d1);
					if (descrs_met) {
						goto descrs_met_flag;
					}
					assert(d1 != NULL);
					d1->free = NULL;
					descrs_met = descrs_met || get_next_descr(sle1, &descr_i_1, d1, d2);
					ff_mark = false; ff_pin = false; ff_i = 0;
				}
//				ff_mark = (bool)(d1->mark_bits[ff_i / bit_in_size_t] & ((long)1 << (bit_in_size_t - (ff_i % bit_in_size_t) - 1)));
//				ff_pin = (bool)(d1->pin_bits[ff_i / bit_in_size_t] & ((long)1 << (bit_in_size_t - (ff_i % bit_in_size_t) - 1)));
				ff_mark = get_object_mark((void *)((size_t)d1->page + d1->obj_size * ff_i), true);
				ff_pin = get_object_mark((void *)((size_t)d1->page + d1->obj_size * ff_i), false);
				assert(ff_mark != -1 && ff_pin != -1);
//				if (ff_mark) {
				if (ff_mark == 1) {
					marked_objects_in_sweep++;
//				} else if (ff_pin) {
				} else if (ff_pin == 1) {
					pined_objects_in_sweep++;
				} else {
					to_place = (void *)((size_t)d1->page + d1->obj_size * ff_i);
					memset(to_place, 0, d1->obj_size);
					goto move_object;
				}
				ff_i++;
			}
			move_object:
			if (ff_i >= sf_i && descrs_met) {
				assert(d1 == d2);
				clear_page_flags(d1);
//				d1->free = (void *)((size_t)d1->page + d1->obj_size * (max(ff_i, last_occupied_object + 1)));
				goto descrs_met_flag;
			}
			void * from_place = (void *)((size_t)d2->page + d2->obj_size * sf_i);
			memcpy(to_place, from_place, d1->obj_size);
			assert(((Object *)((size_t)to_place + d1->obj_size - sizeof(Object)))->meta != NULL);
			assert(((Object *)((size_t)from_place + d2->obj_size - sizeof(Object)))->meta ==
					       ((Object *)((size_t)to_place + d1->obj_size - sizeof(Object)))->meta);
			// TODO change to forward pointer!
			memset(from_place, 0, d1->obj_size);
			add_moved_record(from_place, to_place);
			set_forward_pointer(from_place, to_place);

			printf("moved %p to %p %i %i\n", from_place, to_place, sf_i, ff_i);
			moved_count++;
			sf_mark = false, ff_mark = true, ff_i++, sf_i--;
		}
		descrs_met_flag:
		continue;
	}
	gcmalloc_fix_ptr();
	unlock_all_mutexes();
	printf("moved count == %zu\n", moved_count);
	printf("fixed count == %zu\n", fixed_count);
	printf("marked count == %zu %zu pined count = %zu\n", marked_objects, marked_objects_in_sweep, pined_objects_in_sweep);
	assert(marked_objects_in_sweep >= marked_objects - 1);
	moved_count = fixed_count = marked_objects = marked_objects_in_sweep = pined_objects_in_sweep = 0;
}