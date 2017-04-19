#ifndef ALLOCGC_INCREMENTAL_GC_HPP
#define ALLOCGC_INCREMENTAL_GC_HPP

#include <atomic>

#include <liballocgc/details/collectors/gc_core.hpp>
#include <liballocgc/details/collectors/packet_manager.hpp>
#include <liballocgc/details/collectors/remset.hpp>
#include <liballocgc/details/collectors/marker.hpp>
#include <liballocgc/details/utils/utility.hpp>

namespace allocgc { namespace details { namespace collectors {

class gc_cms : public gc_core, private utils::noncopyable, private utils::nonmovable
{
public:
    gc_cms();

    gc_alloc::response allocate(const gc_alloc::request& rqst);

    void commit(const gc_alloc::response& rsp);
    void commit(const gc_alloc::response& rsp, const gc_type_meta* type_meta);

    void  wbarrier(gc_handle& dst, const gc_handle& src);

    gc_run_stat gc(const gc_options& options);

    gc_info info() const;
private:
    gc_run_stat start_marking_phase();
    gc_run_stat sweep();

    remset m_remset;
    size_t m_threads_available;
    gc_phase m_phase;
};

}}}

#endif //ALLOCGC_INCREMENTAL_GC_HPP
