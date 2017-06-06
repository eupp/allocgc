#ifndef ALLOCGC_INCREMENTAL_GC_HPP
#define ALLOCGC_INCREMENTAL_GC_HPP

#include <mutex>

#include <liballocgc/details/collectors/gc_core.hpp>
#include <liballocgc/details/collectors/packet_manager.hpp>
#include <liballocgc/details/collectors/remset.hpp>
#include <liballocgc/details/collectors/marker.hpp>
#include <liballocgc/details/utils/utility.hpp>

namespace allocgc { namespace details { namespace collectors {

class gc_cms : public gc_core<gc_cms>, private utils::noncopyable, private utils::nonmovable
{
public:
    gc_cms();

    gc_alloc::response allocate(const gc_alloc::request& rqst);

    void commit(const gc_alloc::response& rsp);
    void commit(const gc_alloc::response& rsp, const gc_type_meta* type_meta);

    void  wbarrier(gc_handle& dst, const gc_handle& src);

    gc_runstat gc_impl(const gc_options& options);

    gc_info info() const override;
private:
    gc_runstat start_marking_phase();
    gc_runstat sweep();

    remset m_remset;
    std::mutex m_mutex;
    gc_phase m_phase;
};

}}}

#endif //ALLOCGC_INCREMENTAL_GC_HPP
