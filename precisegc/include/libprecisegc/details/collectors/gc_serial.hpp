#ifndef DIPLOMA_SERIAL_GC_HPP
#define DIPLOMA_SERIAL_GC_HPP

#include <memory>

#include <libprecisegc/details/collectors/gc_core.hpp>
#include <libprecisegc/details/collectors/packet_manager.hpp>
#include <libprecisegc/details/collectors/marker.hpp>
#include <libprecisegc/details/gc_strategy.hpp>
#include <libprecisegc/details/gc_heap.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details { namespace collectors {

class gc_serial : public gc_core, private utils::noncopyable, private utils::nonmovable
{
public:
    gc_serial(size_t threads_available, const thread_descriptor& main_thrd_descr);

    void  wbarrier(gc_handle& dst, const gc_handle& src) override;

    gc_run_stats gc(const gc_options& options) override;

    gc_info info() const;
private:
    gc_run_stats sweep();

    size_t m_threads_available;
};

}}}

#endif //DIPLOMA_SERIAL_GC_HPP
