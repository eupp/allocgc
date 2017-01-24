#ifndef DIPLOMA_HEAP_H
#define DIPLOMA_HEAP_H

#include <utility>
#include <atomic>
#include <cstddef>
#include <mutex>
#include <memory>
#include <unordered_map>

#include <libprecisegc/details/allocators/gc_lo_allocator.hpp>
#include <libprecisegc/details/allocators/gc_so_allocator.hpp>

#include <libprecisegc/details/compacting/forwarding.hpp>

#include <libprecisegc/details/threads/world_snapshot.hpp>

#include <libprecisegc/details/utils/safepoint_lock.hpp>
#include <libprecisegc/details/utils/dummy_mutex.hpp>
#include <libprecisegc/details/utils/utility.hpp>

#include <libprecisegc/details/allocators/gc_alloc_messaging.hpp>
#include <libprecisegc/details/gc_interface.hpp>
#include <libprecisegc/details/constants.hpp>

namespace precisegc { namespace details {

class gc_heap : public utils::noncopyable, public utils::nonmovable
{
    typedef allocators::gc_so_allocator so_alloc_t;
    typedef allocators::gc_lo_allocator lo_alloc_t;
    typedef compacting::forwarding forwarding;
public:
    gc_heap();

    allocators::gc_alloc_response allocate(const allocators::gc_alloc_request& rqst);

    gc_heap_stat collect(const threads::world_snapshot& snapshot, gc_gen gen, size_t threads_available);
private:
    struct tlab_t
    {
        size_t size;
        so_alloc_t alloc;
    };

    typedef std::unordered_map<std::thread::id, tlab_t> tlab_map_t;

    allocators::gc_alloc_response allocate_on_tlab(const allocators::gc_alloc_request& rqst);
    tlab_t& get_tlab();

    static const size_t TLAB_SIZE;

    lo_alloc_t m_loa;
    so_alloc_t m_old_soa;
    tlab_map_t m_tlab_map;
    std::mutex m_tlab_map_mutex;
};

}}

#endif //DIPLOMA_HEAP_H
