#ifndef ALLOCGC_SERIAL_GC_HPP
#define ALLOCGC_SERIAL_GC_HPP

#include <mutex>

#include <liballocgc/details/collectors/gc_core.hpp>
#include <liballocgc/details/collectors/packet_manager.hpp>
#include <liballocgc/details/collectors/marker.hpp>
#include <liballocgc/details/collectors/gc_heap.hpp>
#include <liballocgc/details/utils/utility.hpp>

namespace allocgc { namespace details { namespace collectors {

class gc_serial : public gc_core<gc_serial>, private utils::noncopyable, private utils::nonmovable
{
public:
    gc_serial();

    void  wbarrier(gc_handle& dst, const gc_handle& src);

    gc_runstat gc_impl(const gc_options& options);

    gc_info info() const override;
private:
    gc_runstat sweep();
};

}}}

#endif //ALLOCGC_SERIAL_GC_HPP
