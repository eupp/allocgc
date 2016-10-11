#ifndef DIPLOMA_INCREMENTAL_GC_MOCK_HPP
#define DIPLOMA_INCREMENTAL_GC_MOCK_HPP

#include <gmock/gmock.h>

#include <libprecisegc/details/gc_strategy.hpp>
#include <libprecisegc/details/gc_hooks.hpp>

class incremental_gc_mock : public precisegc::details::gc_strategy
{
    typedef precisegc::details::byte byte;
    typedef precisegc::details::gc_type_meta type_meta;
    typedef precisegc::details::gc_word gc_word;
    typedef precisegc::details::gc_alloc_descriptor gc_alloc_descriptor;
    typedef precisegc::details::initiation_point_type initation_point_type;
    typedef precisegc::details::gc_options gc_options;
    typedef precisegc::details::gc_run_stats gc_stats;
    typedef precisegc::details::gc_phase gc_phase;
    typedef precisegc::details::gc_info gc_info;
public:
    MOCK_METHOD3(allocate, gc_alloc_descriptor(size_t obj_size, size_t obj_cnt, const type_meta* tmeta));
    MOCK_METHOD1(commit, void(const gc_alloc_descriptor&));

    MOCK_METHOD1(rbarrier, byte*(const gc_word&));
    MOCK_METHOD2(wbarrier, void(gc_word&, const gc_word&));

    MOCK_METHOD2(interior_wbarrier, void(gc_word& handle, ptrdiff_t offset));

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
