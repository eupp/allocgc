#ifndef DIPLOMA_HEAP_H
#define DIPLOMA_HEAP_H

#include <utility>
#include <atomic>
#include <cstddef>
#include <mutex>

#include "forwarding.h"
#include "object_meta.h"
#include "allocators/bucket_allocator.hpp"
#include "allocators/pow2_bucket_policy.h"
#include "allocators/paged_allocator.h"
#include "allocators/fixed_block_cache.h"
#include "constants.hpp"
#include "managed_pool_chunk.hpp"
#include "libprecisegc/details/utils/utility.hpp"
#include "gc_hooks.hpp"

namespace precisegc { namespace details {

class gc_heap : public utils::noncopyable, public utils::nonmovable
{
    static const size_t SEGREGATED_STORAGE_SIZE = (POINTER_BITS_CNT - RESERVED_BITS_CNT - 1);
    static const size_t MIN_ALLOC_SIZE_BITS = 6;
    static const size_t MAX_ALLOC_SIZE_BITS = MIN_ALLOC_SIZE_BITS + SEGREGATED_STORAGE_SIZE;

    typedef allocators::bucket_allocator<
            managed_pool_chunk,
            allocators::paged_allocator,
            allocators::paged_allocator,
            allocators::pow2_bucket_policy<MIN_ALLOC_SIZE_BITS, MAX_ALLOC_SIZE_BITS>,
            std::mutex
        > alloc_t;

    typedef intrusive_forwarding forwarding;
public:
    gc_heap(gc_compacting compacting);
    gc_heap(const gc_heap&&) = delete;
    gc_heap& operator=(const gc_heap&&) = delete;


    managed_ptr allocate(size_t size);

    void sweep();

    size_t size() const noexcept;
private:
    forwarding compact_memory();
    void fix_pointers(const forwarding& frwd);

    alloc_t m_alloc;
    std::atomic<size_t> m_size;
    gc_compacting m_compacting;
};

}}

#endif //DIPLOMA_HEAP_H
