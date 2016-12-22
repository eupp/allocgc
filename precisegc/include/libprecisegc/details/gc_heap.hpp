#ifndef DIPLOMA_HEAP_H
#define DIPLOMA_HEAP_H

#include <utility>
#include <atomic>
#include <cstddef>
#include <mutex>
#include <memory>
#include <unordered_map>

#include <libprecisegc/details/collectors/traceable_object_meta.hpp>

#include <libprecisegc/details/allocators/mlo_allocator.hpp>
#include <libprecisegc/details/allocators/mso_allocator.hpp>

#include <libprecisegc/details/compacting/forwarding.hpp>

#include <libprecisegc/details/threads/world_snapshot.hpp>

#include <libprecisegc/details/utils/safepoint_lock.hpp>
#include <libprecisegc/details/utils/dummy_mutex.hpp>
#include <libprecisegc/details/utils/utility.hpp>

#include <libprecisegc/details/gc_alloc_messaging.hpp>
#include <libprecisegc/details/gc_interface.hpp>
#include <libprecisegc/details/constants.hpp>

namespace precisegc { namespace details {

class gc_heap : public utils::noncopyable, public utils::nonmovable
{
    typedef allocators::mso_allocator mso_alloc_t;
    typedef allocators::mlo_allocator mlo_alloc_t;
    typedef compacting::forwarding forwarding;
public:
    gc_heap(gc_compacting compacting);

    gc_alloc_response allocate(const gc_alloc_request& rqst);

    gc_heap_stat collect(const threads::world_snapshot& snapshot, size_t threads_available);
private:
    typedef std::unordered_map<std::thread::id, mso_alloc_t> tlab_map_t;

    gc_alloc_response allocate_on_tlab(const gc_alloc_request& rqst);
    mso_alloc_t& get_tlab();

    mlo_alloc_t m_loa;
    tlab_map_t m_tlab_map;
    std::mutex m_tlab_map_mutex;
    gc_compacting m_compacting;
};

}}

#endif //DIPLOMA_HEAP_H
