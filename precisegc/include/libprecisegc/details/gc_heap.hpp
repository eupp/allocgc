#ifndef DIPLOMA_HEAP_H
#define DIPLOMA_HEAP_H

#include <utility>
#include <atomic>
#include <cstddef>
#include <mutex>
#include <memory>
#include <unordered_map>

#include <libprecisegc/gc_alloc.hpp>
#include <libprecisegc/gc_factory.hpp>
#include <libprecisegc/details/gc_interface.hpp>
#include <libprecisegc/details/constants.hpp>

#include <libprecisegc/details/allocators/gc_lo_allocator.hpp>
#include <libprecisegc/details/allocators/gc_so_allocator.hpp>

#include <libprecisegc/details/compacting/forwarding.hpp>

#include <libprecisegc/details/threads/world_snapshot.hpp>
#include <libprecisegc/details/utils/safepoint_lock.hpp>
#include <libprecisegc/details/utils/dummy_mutex.hpp>

#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details {

class gc_heap : public utils::noncopyable, public utils::nonmovable
{
    typedef allocators::gc_so_allocator mso_alloc_t;
    typedef allocators::gc_lo_allocator mlo_alloc_t;
    typedef compacting::forwarding forwarding;
public:
    gc_heap(const gc_factory::options& opt);

    gc_alloc::response allocate(const gc_alloc::request& rqst);

    gc_heap_stat collect(const threads::world_snapshot& snapshot, size_t threads_available,
                             collectors::static_root_set* static_roots);
private:
    typedef std::unordered_map<std::thread::id, mso_alloc_t> tlab_map_t;

    gc_alloc::response allocate_on_tlab(const gc_alloc::request& rqst);
    mso_alloc_t& get_tlab();

    mlo_alloc_t m_loa;
    tlab_map_t  m_tlab_map;
    std::mutex  m_tlab_map_mutex;
    bool        m_conservative_mode;
};

}}

#endif //DIPLOMA_HEAP_H
