#ifndef ALLOCGC_SERIAL_GC_MOCK_HPP
#define ALLOCGC_SERIAL_GC_MOCK_HPP

#include <gmock/gmock.h>

#include <liballocgc/gc_strategy.hpp>

//class serial_gc_mock : public allocgc::details::gc_strategy
//{
//    typedef allocgc::details::byte byte;
//    typedef allocgc::details::gc_cell gc_cell;
//    typedef allocgc::details::gc_handle gc_handle;
//    typedef allocgc::details::gc_type_meta gc_type_meta;
//    typedef allocgc::details::allocators::gc_alloc_response gc_alloc_response;
//    typedef allocgc::details::initiation_point_type initation_point_type;
//    typedef allocgc::details::gc_options gc_options;
//    typedef allocgc::details::gc_run_stats gc_stats;
//    typedef allocgc::details::gc_phase gc_phase;
//    typedef allocgc::details::gc_info gc_info;
//public:
//    MOCK_METHOD3(allocate, gc_alloc_response (size_t obj_size, size_t obj_cnt, const gc_type_meta* tmeta));
//
//    MOCK_METHOD1(commit, void(gc_cell&));
//    MOCK_METHOD2(commit, void(gc_cell&, const gc_type_meta*));
//
//    MOCK_METHOD1(rbarrier, byte*(const gc_handle&));
//    MOCK_METHOD2(wbarrier, void(gc_handle&, const gc_handle&));
//
//    MOCK_METHOD2(interior_wbarrier, void(gc_handle& handle, ptrdiff_t offset));
//
//    MOCK_METHOD1(pin, byte*(const gc_handle& handle));
//    MOCK_METHOD1(unpin, void(byte* ptr));
//
//    MOCK_METHOD2(compare, bool(const gc_handle& a, const gc_handle& b));
//
//    MOCK_METHOD1(gc, gc_stats(const gc_options&));
//
//    gc_info info() const override
//    {
//        static gc_info inf = {
//                .incremental_flag                = false,
//                .support_concurrent_marking      = false,
//                .support_concurrent_collecting   = false
//        };
//        return inf;
//    }
//};

#endif //ALLOCGC_SERIAL_GC_MOCK_HPP
