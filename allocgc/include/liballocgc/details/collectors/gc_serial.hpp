#ifndef ALLOCGC_SERIAL_GC_HPP
#define ALLOCGC_SERIAL_GC_HPP

#include <mutex>

#include <liballocgc/details/collectors/gc_core.hpp>
#include <liballocgc/details/collectors/packet_manager.hpp>
#include <liballocgc/details/collectors/marker.hpp>
#include <liballocgc/details/collectors/gc_heap.hpp>
#include <liballocgc/details/utils/utility.hpp>

namespace allocgc { namespace details { namespace collectors {

struct gc_serial_tag {};

class gc_serial : public gc_core<gc_serial_tag>, public gc_launcher, private utils::noncopyable, private utils::nonmovable
{
public:
    gc_serial();

    void  wbarrier(gc_handle& dst, const gc_handle& src);

    gc_run_stat gc(const gc_options& options) override;

    gc_info info() const;
private:
    gc_run_stat sweep();

    std::mutex m_mutex;
};

}}}

#endif //ALLOCGC_SERIAL_GC_HPP
