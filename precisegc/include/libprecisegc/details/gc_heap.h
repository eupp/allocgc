#ifndef DIPLOMA_HEAP_H
#define DIPLOMA_HEAP_H

#include <utility>
#include <atomic>
#include <cstddef>

#include "forwarding.h"
#include "object_meta.h"
#include "mutex.h"
#include "allocators/bucket_allocator.h"
#include "allocators/pow2_bucket_policy.h"
#include "allocators/paged_allocator.h"
#include "allocators/fixed_block_cache.h"
#include "allocators/constants.h"
#include "managed_pool_chunk.h"
#include "util.h"

namespace precisegc { namespace details {

class gc_heap : public noncopyable, public nonmovable
{
    static const size_t SEGREGATED_STORAGE_SIZE = (POINTER_BITS_CNT - RESERVED_BITS_CNT - 1);
    static const size_t MIN_ALLOC_SIZE_BITS = 4;
    static const size_t MAX_ALLOC_SIZE_BITS = MIN_ALLOC_SIZE_BITS + SEGREGATED_STORAGE_SIZE;

    typedef allocators::bucket_allocator<
            managed_pool_chunk,
            allocators::paged_allocator,
            allocators::paged_allocator,
            allocators::pow2_bucket_policy<MIN_ALLOC_SIZE_BITS, MAX_ALLOC_SIZE_BITS>,
            mutex
        > alloc_t;

    typedef intrusive_forwarding forwarding;
public:
    static gc_heap& instance()
    {
        static gc_heap h;
        return h;
    }

    managed_cell_ptr allocate(size_t size);

    void compact();

    size_t size() noexcept;
private:
    static size_t align_size(size_t size);

    gc_heap();
    gc_heap(const gc_heap&&) = delete;
    gc_heap& operator=(const gc_heap&&) = delete;

    forwarding compact_memory();
    void fix_pointers(const forwarding& frwd);
    void sweep();

    //void fix_pointers(const forwarding_list& forwarding);

    alloc_t m_alloc;
    std::atomic<size_t> m_size;
};

}}

#endif //DIPLOMA_HEAP_H
