#ifndef ALLOCGC_SERIAL_GC_HPP
#define ALLOCGC_SERIAL_GC_HPP

#include <memory>

#include <liballocgc/details/collectors/gc_core.hpp>
#include <liballocgc/details/collectors/packet_manager.hpp>
#include <liballocgc/details/collectors/marker.hpp>
#include <liballocgc/details/gc_heap.hpp>
#include <liballocgc/details/utils/utility.hpp>

namespace allocgc { namespace details { namespace collectors {

class gc_serial : public gc_core, private utils::noncopyable, private utils::nonmovable
{
public:
    gc_serial();

    void  wbarrier(gc_handle& dst, const gc_handle& src);

    gc_run_stat gc(const gc_options& options);

    gc_info info() const;
private:
    gc_run_stat sweep();

    size_t m_threads_available;
};

}}}

#endif //ALLOCGC_SERIAL_GC_HPP
