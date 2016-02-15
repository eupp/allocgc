#include "index_tree.h"

#include <cassert>
#include <cstring>
#include <sys/mman.h>

#include "logging.h"

using namespace precisegc::details;

const size_t SystemBitSize = precisegc::details::SYSTEM_POINTER_BITS_COUNT;
const size_t CellBits = precisegc::details::MEMORY_CELL_SIZE_BITS;

namespace _GC_ {
	static pthread_mutex_t index_tree_mutex = PTHREAD_MUTEX_INITIALIZER;

    class ITMemoryPool {
    private:
	    struct ITMemoryPoolElement {
	        ITMemoryPoolElement * prev, * next;
		    void * data;
		    size_t size;
	    };
		ITMemoryPoolElement * free_list, * list_begin;
		size_t elementSize, free_list_size;

		ITMemoryPool () : free_list(NULL), list_begin(NULL), free_list_size(0) {
			elementSize = (ITOtherLevelsSize + sizeof(ITMemoryPoolElement)) * ITMemoryPoolUnits;
		}
//	    ~ITMemoryPool() {}
//	    ITMemoryPool (const ITMemoryPool&);
	    ITMemoryPool& operator= (const ITMemoryPool&);

    public:
		static ITMemoryPool& getInstance () {
			static thread_local ITMemoryPool instance;
			return instance;
		}

	    void * allocateNew (void) {
		    ITMemoryPoolElement * res = NULL;
		    if (free_list == NULL) {
				void * new_memory = mmap(NULL, elementSize,
			                           PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
				memset(new_memory, 0, elementSize);
			    res = (ITMemoryPoolElement *)((size_t)new_memory + ITOtherLevelsSize);
				*res = {NULL, list_begin, new_memory, 0};
				size_t free_els = (size_t)res + sizeof(ITMemoryPoolElement);
				for (int i = 1; i < ITMemoryPoolUnits; i++, free_els += ITOtherLevelsSize + sizeof(ITMemoryPoolElement)) {
					ITMemoryPoolElement * new_free_list_el = (ITMemoryPoolElement *)(free_els + ITOtherLevelsSize);
					*new_free_list_el = {NULL, free_list, (void *)(free_els), 0};
					if (free_list) { free_list->prev = new_free_list_el; }
					free_list = new_free_list_el;
				}
				free_list_size += ITMemoryPoolUnits - 1;
			} else {
			    assert(free_list_size != 0);
			    res = free_list; free_list = free_list->next;
				if (free_list) {
                    free_list->prev = NULL;
                }
			    free_list_size--;
			    res->next = list_begin;
		    }
		    if (list_begin) { list_begin->prev = res; }
		    list_begin = res;
		    return res->data;
	    }

		bool singleElementInMemoryPoolElement (void *ptr) {
			return ((ITMemoryPoolElement *)((size_t)ptr + ITOtherLevelsSize))->size == 1;
		}

	    void increaseLevelSize (void *ptr) {
		    ((ITMemoryPoolElement *)((size_t)ptr + ITOtherLevelsSize))->size++;
	    }

	    void deallocate (void * ptr) {
		    ITMemoryPoolElement * del = (ITMemoryPoolElement *)((size_t)ptr + ITOtherLevelsSize);
		    free_list_size++;
		    assert (del->size == 0 || del->size == 1);
		    if (del->next) { del->next->prev = del->prev; }
		    if (del->prev) { del->prev->next = del->next; }
		    del->next = free_list;
		    del->prev = NULL; del->size = 0;
		    if (free_list) { free_list->prev = del; }
		    memset(del->data, 0, ITOtherLevelsSize);
	    }
    };

	ITMemoryPool ITOtherLevelsPool = ITMemoryPool::getInstance();
	size_t *tree_level_one = (size_t *) mmap(NULL, (((size_t) 1) << ITFirstLevelBits) * sizeof(size_t),
                                             PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);

    void IT_index_new_cell(void *cell, page_descriptor *page_decr) {
	    pthread_mutex_lock(&index_tree_mutex);
		size_t bits_curr = SystemBitSize - ITFirstLevelBits;
		size_t i_l = (size_t) cell >> bits_curr;
		size_t *level = tree_level_one;

	    for (int i = 0; i < (int) ITLevelCount - 1; i++) {
			level[i_l] = level[i_l] != 0 ? level[i_l] : (size_t) ITOtherLevelsPool.allocateNew();
			ITOtherLevelsPool.increaseLevelSize((void *) level[i_l]);
			assert((void *) level[i_l] != NULL);
			level = reinterpret_cast <size_t *> (level[i_l]);
			bits_curr -= ITOtherLevelsBits;
			i_l = ((size_t) cell >> (bits_curr)) & (((size_t) 1 << (ITOtherLevelsBits)) - 1);
	    }
	    if (i_l == 509) {
		    i_l = 509;
	    }
//		assert(level[i_l] == 0);
		assert((size_t) cell - (size_t) page_decr->page() < page_decr->page_size());
		level[i_l] = (size_t) page_decr;
	    pthread_mutex_unlock(&index_tree_mutex);
    }

    size_t *IT_iterate(void *ptr, size_t *bits_curr, size_t *i_l, size_t *level) {
	    for (int i = 0; i < (int) ITLevelCount - 1; i++) {
		    if ((void *) level[*i_l] == NULL) { return NULL; }
		    level = reinterpret_cast <size_t *> (level[*i_l]);
		    *bits_curr -= ITOtherLevelsBits;
		    *i_l = ((size_t) ptr >> (*bits_curr)) & (((size_t) 1 << (ITOtherLevelsBits)) - 1);
	    }
	    return level;
    }

	/* return a page ptr points to descriptor */
    void *IT_get_page_descr(void* ptr) {
		pthread_mutex_lock(&index_tree_mutex);

//        logging::debug() << "Thread " << pthread_self() << " locks index tree mutex ";

	    size_t bits_curr = SystemBitSize - ITFirstLevelBits;
	    size_t i_l = (size_t) ptr >> bits_curr;
	    size_t *level = IT_iterate(ptr, &bits_curr, &i_l, tree_level_one);
		assert(level == NULL || level[i_l] == 0
		       || (size_t)ptr - (size_t)((page_descriptor *)level[i_l])->page() < ((page_descriptor *)level[i_l])->page_size());
		void * res = level != NULL ? (void *) level[i_l] : NULL;
	    pthread_mutex_unlock(&index_tree_mutex);

//        logging::debug() << "Thread " << pthread_self() << " release index tree mutex ";

		return res;
    }

	/* remove index */
    void IT_remove_index(void *cell) {
		pthread_mutex_lock(&index_tree_mutex);
	    size_t bits_curr = SystemBitSize - CellBits - ITFirstLevelBits;
	    size_t i_l = (size_t) cell >> bits_curr, prev_i_l = 0;
		size_t levels[ITLevelCount]; levels[0] = i_l;
	    size_t * level = tree_level_one, * prev_level = NULL;
		for (int i = 0; i < (int) ITLevelCount - 1; i++) {
			if ((void *) level[i_l] == NULL) {
//				myfile << "IT_remove_index : doesn't contain cell " << cell << endl;
				pthread_mutex_unlock(&index_tree_mutex);
				return;
			}
			size_t * next_level = reinterpret_cast <size_t *> (level[i_l]);
			// deallocate level if it contains only one element;
			if (ITOtherLevelsPool.singleElementInMemoryPoolElement(level)) {
				ITOtherLevelsPool.deallocate(level);
				if (prev_level != NULL) { prev_level[prev_i_l] = 0; }
			}
			prev_level = level; level = next_level;
			bits_curr -= ITOtherLevelsBits;
			prev_i_l = i_l;
			i_l = ((size_t) cell >> (bits_curr)) & (((size_t) 1 << (ITOtherLevelsBits)) - 1);
		}

		if (level) { level[i_l] = 0; }
		pthread_mutex_unlock(&index_tree_mutex);
	}

    void IT_print_tree(void) {
	    pthread_mutex_lock(&index_tree_mutex);
	    printf("tree:\n");
	    size_t *level = tree_level_one, *level2, *level3;
	    for (size_t i = 0; i < (((size_t) 1) << ITOtherLevelsBits); i++) {
		    if (level[i] == 0) { continue; }
		    level2 = reinterpret_cast<size_t *> ((void *) level[i]);
		    for (size_t j = 0; j < (((size_t) 1) << ITOtherLevelsBits); j++) {
			    if (level2[j] == 0) { continue; }
			    level3 = reinterpret_cast<size_t *> ((void *) level2[j]);
			    for (size_t k = 0; k < (((size_t) 1) << ITOtherLevelsBits); k++) {
				    if (level3[k] == 0) { continue; }
				    printf("%zu ", level3[k]);
			    }
			    printf("\n\n");
		    }
		    printf("\n\n\n");
	    }
	    pthread_mutex_unlock(&index_tree_mutex);
    }
}