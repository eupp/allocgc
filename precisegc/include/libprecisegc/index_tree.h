#pragma once
#include "gc_malloc.h"
////////////////
// INDEX TREE //
////////////////

#ifndef _GC_IT_LEVEL_COUNT
#define _GC_IT_LEVEL_COUNT ((size_t) 3)
#endif

#include "details/page_descriptor.h"

namespace _GC_ {
	enum {
		ITLevelCount      = _GC_IT_LEVEL_COUNT,
		ITBitsInUse       = (SystemBitSize - CellBits),
		ITFirstLevelBits  = ITBitsInUse -  (size_t)(ITBitsInUse / ITLevelCount) * (ITLevelCount - 1),
		ITOtherLevelsBits = ((size_t)(ITBitsInUse / ITLevelCount)),
		ITOtherLevelsSize = ((size_t)1 << ITOtherLevelsBits) * sizeof(size_t),
		ITMemoryPoolUnits = 32
	};
    extern size_t *tree_level_one;

    void IT_index_new_cell(void *cell, precisegc::details::page_descriptor *page_decr);
    size_t *IT_iterate(void *ptr, size_t *bits_curr, size_t *i_l, size_t *level);
    void *IT_get_page_descr(void *ptr);
    void IT_remove_index(void *cell);
    void IT_print_tree(void);
};