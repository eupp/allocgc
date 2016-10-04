#ifndef DIPLOMA_INCREMENTAL_GC_MOCK_HPP
#define DIPLOMA_INCREMENTAL_GC_MOCK_HPP

#include <gmock/gmock.h>

#include <libprecisegc/details/gc_strategy.hpp>
#include <libprecisegc/details/gc_hooks.hpp>

class incremental_gc_mock : public precisegc::details::gc_strategy
{
    typedef precisegc::details::byte byte;
    typedef precisegc::details::gc_handle gc_handle;
    typedef precisegc::details::indexed_managed_object managed_ptr;
    typedef precisegc::details::gc_pointer_type gc_pointer_type;
    typedef precisegc::details::initiation_point_type initation_point_type;
    typedef precisegc::details::gc_options gc_options;
    typedef precisegc::details::gc_run_stats gc_stats;
    typedef precisegc::details::gc_phase gc_phase;
    typedef precisegc::details::gc_info gc_info;
public:
    MOCK_METHOD1(allocate, gc_pointer_type(size_t));
    MOCK_METHOD1(commit, void(const managed_ptr&));

    MOCK_METHOD1(rbarrier, byte*(const gc_handle&));
    MOCK_METHOD2(wbarrier, void(gc_handle&, const gc_handle&));

    MOCK_METHOD2(interior_wbarrier, void(gc_handle& handle, ptrdiff_t offset));

    MOCK_METHOD1(pin, byte*(const gc_handle& handle));
    MOCK_METHOD1(unpin, void(byte* ptr));

    MOCK_METHOD2(compare, bool(const gc_handle& a, const gc_handle& b));

    MOCK_METHOD1(gc, gc_stats(const gc_options&));

    gc_info info() const override
    {
        static gc_info inf = {
                .incremental_flag                = true,
                .support_concurrent_marking    = false,
                .support_concurrent_collecting   = false
        };
        return inf;
    }
};

#endif //DIPLOMA_INCREMENTAL_GC_MOCK_HPP
