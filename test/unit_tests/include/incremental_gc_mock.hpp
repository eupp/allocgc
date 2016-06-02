#ifndef DIPLOMA_INCREMENTAL_GC_MOCK_HPP
#define DIPLOMA_INCREMENTAL_GC_MOCK_HPP

#include <gmock/gmock.h>

#include <libprecisegc/details/gc_interface.hpp>

class incremental_gc_mock : public precisegc::details::incremental_gc_interface
{
    typedef precisegc::details::byte byte;
    typedef precisegc::details::atomic_byte_ptr atomic_byte_ptr;
    typedef precisegc::details::managed_ptr managed_ptr;
    typedef precisegc::details::initation_point_type initation_point_type;
    typedef precisegc::details::gc_stat gc_stat;
    typedef precisegc::details::gc_phase gc_phase;
    typedef precisegc::details::incremental_gc_ops incremental_gc_ops;
public:
    MOCK_METHOD1(allocate, managed_ptr(size_t));
    MOCK_METHOD1(rbarrier, byte*(const atomic_byte_ptr&));
    MOCK_METHOD2(wbarrier, void(atomic_byte_ptr&, const atomic_byte_ptr&));
    MOCK_METHOD1(initation_point, void(initation_point_type));
    MOCK_CONST_METHOD0(info, gc_info(void));
    MOCK_METHOD0(gc, void(void));
    MOCK_CONST_METHOD0(phase, gc_phase());
    MOCK_METHOD1(incremental_gc, void(const incremental_gc_ops&));
};

#endif //DIPLOMA_INCREMENTAL_GC_MOCK_HPP
