#ifndef DIPLOMA_INCREMENTAL_GC_HPP
#define DIPLOMA_INCREMENTAL_GC_HPP

#include <atomic>

#include <libprecisegc/details/collectors/gc_core.hpp>
#include <libprecisegc/details/collectors/packet_manager.hpp>
#include <libprecisegc/details/collectors/remset.hpp>
#include <libprecisegc/details/collectors/marker.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/gc_strategy.hpp>

namespace precisegc { namespace details { namespace collectors {

class gc_incremental : public gc_core, private utils::noncopyable, private utils::nonmovable
{
public:
    gc_incremental(size_t threads_available, const thread_descriptor& main_thrd_descr);

    gc_alloc::response allocate(const gc_alloc::request& rqst) override;

    void commit(const gc_alloc::response& rsp) override;
    void commit(const gc_alloc::response& rsp, const gc_type_meta* type_meta) override;

    void  wbarrier(gc_handle& dst, const gc_handle& src) override;

    gc_run_stats gc(const gc_options& options) override;

    gc_info info() const;
private:
    gc_run_stats start_marking();
    gc_run_stats sweep();

    packet_manager m_packet_manager;
    remset m_remset;
    marker m_marker;
    size_t m_threads_available;
    gc_phase m_phase;
};

}}}

#endif //DIPLOMA_INCREMENTAL_GC_HPP
