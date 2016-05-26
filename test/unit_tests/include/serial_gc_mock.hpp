#ifndef DIPLOMA_SERIAL_GC_MOCK_HPP
#define DIPLOMA_SERIAL_GC_MOCK_HPP

#include <gmock/gmock.h>

#include <libprecisegc/details/gc_interface.hpp>

class serial_gc_mock : public precisegc::details::serial_gc_interface
{
    typedef precisegc::details::byte byte;
    typedef precisegc::details::atomic_byte_ptr atomic_byte_ptr;
    typedef precisegc::details::managed_ptr managed_ptr;
    typedef precisegc::details::gc_untyped_ptr gc_untyped_ptr;
public:
    MOCK_METHOD1(allocate, managed_ptr(size_t));
    MOCK_METHOD2(write_barrier, void(gc_untyped_ptr&, const gc_untyped_ptr&));
    MOCK_CONST_METHOD1(load_ptr, byte*(const atomic_byte_ptr&));
    MOCK_CONST_METHOD2(store_ptr, void(atomic_byte_ptr&, byte*));
    MOCK_CONST_METHOD0(stat, gc_stat(void));
    MOCK_METHOD0(gc, void(void));
};

#endif //DIPLOMA_SERIAL_GC_MOCK_HPP
