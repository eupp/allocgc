#ifndef DIPLOMA_HEAP_H
#define DIPLOMA_HEAP_H

#include <utility>
#include <atomic>
#include <cstddef>
#include <mutex>
#include <memory>

#include <libprecisegc/details/allocators/default_allocator.hpp>
#include <libprecisegc/details/allocators/page_allocator.hpp>
#include <libprecisegc/details/allocators/bucket_allocator.hpp>
#include <libprecisegc/details/allocators/pow2_bucket_policy.h>
#include <libprecisegc/details/threads/world_snapshot.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/forwarding.h>
#include <libprecisegc/details/object_meta.h>
#include <libprecisegc/details/managed_pool_chunk.hpp>
#include <libprecisegc/details/gc_interface.hpp>
#include <libprecisegc/details/constants.hpp>

namespace precisegc { namespace details {

class gc_heap : public utils::noncopyable, public utils::nonmovable
{
    static const size_t SEGREGATED_STORAGE_SIZE = (POINTER_BITS_CNT - ALIGN_BITS_CNT - 1);
    static const size_t MIN_ALLOC_SIZE_BITS = 6;
    static const size_t MAX_ALLOC_SIZE_BITS = MIN_ALLOC_SIZE_BITS + SEGREGATED_STORAGE_SIZE;

    typedef allocators::bucket_allocator<
            managed_pool_chunk,
            allocators::page_allocator,
            allocators::default_allocator,
            allocators::pow2_bucket_policy<MIN_ALLOC_SIZE_BITS, MAX_ALLOC_SIZE_BITS>,
            std::mutex
        > alloc_t;

    typedef intrusive_forwarding forwarding;
public:
    gc_heap(gc_compacting compacting);
    gc_heap(const gc_heap&&) = delete;
    gc_heap& operator=(const gc_heap&&) = delete;


    managed_ptr allocate(size_t size);

    gc_sweep_stat sweep(const threads::world_snapshot& snapshot);
private:
    forwarding compact_memory();
    void fix_pointers(const forwarding& frwd);

    alloc_t m_alloc;
    gc_compacting m_compacting;
};

}}

#endif //DIPLOMA_HEAP_H
