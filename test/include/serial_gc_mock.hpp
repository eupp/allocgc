#ifndef DIPLOMA_SERIAL_GC_MOCK_HPP
#define DIPLOMA_SERIAL_GC_MOCK_HPP

#include <gmock/gmock.h>

#include <libprecisegc/details/gc_strategy.hpp>

class serial_gc_mock : public precisegc::details::serial_gc_strategy
{
    typedef precisegc::details::byte byte;
    typedef precisegc::details::gc_handle gc_handle;
    typedef precisegc::details::managed_ptr managed_ptr;
    typedef precisegc::details::initation_point_type initation_point_type;
    typedef precisegc::details::gc_info gc_info;
public:
    MOCK_METHOD1(allocate, managed_ptr(size_t));

    MOCK_METHOD1(rbarrier, byte*(const gc_handle&));
    MOCK_METHOD2(wbarrier, void(gc_handle&, const gc_handle&));

    MOCK_METHOD2(interior_wbarrier, void(gc_handle& handle, byte* ptr));
    MOCK_METHOD2(interior_shift, void(gc_handle& handle, ptrdiff_t shift));

    MOCK_METHOD1(pin, byte*(const gc_handle& handle));
    MOCK_METHOD1(unpin, void(byte* ptr));

    MOCK_METHOD2(compare, bool(const gc_handle& a, const gc_handle& b));

    MOCK_METHOD1(initation_point, void(initation_point_type));

    MOCK_CONST_METHOD0(info, gc_info(void));

    MOCK_METHOD0(gc, void(void));
};

#endif //DIPLOMA_SERIAL_GC_MOCK_HPP
