#ifndef DIPLOMA_MEMORY_DESCRIPTOR_MOCK_H
#define DIPLOMA_MEMORY_DESCRIPTOR_MOCK_H

#include <gmock/gmock.h>

#include "libprecisegc/details/allocators/gc_pool_descriptor.hpp"

class memory_descriptor_mock : public precisegc::details::gc_memory_descriptor
{
    typedef precisegc::details::byte byte;
    typedef precisegc::details::gc_type_meta gc_type_meta;
    typedef precisegc::details::gc_lifetime_tag gc_lifetime_tag;
    typedef precisegc::details::gc_trace_callback gc_trace_callback;
    typedef precisegc::details::gc_memory_descriptor memory_descriptor;
public:
    MOCK_CONST_METHOD1(get_mark, bool(byte*));
    MOCK_CONST_METHOD1(get_pin, bool(byte*));

    MOCK_METHOD2(set_mark, void(byte* ptr, bool mark));
    MOCK_METHOD2(set_pin, void(byte* ptr, bool pin));

    MOCK_CONST_METHOD1(get_lifetime_tag, gc_lifetime_tag(byte* ptr));

    MOCK_CONST_METHOD1(cell_size, size_t(byte* ptr));
    MOCK_CONST_METHOD1(cell_start, byte*(byte* ptr));

    MOCK_CONST_METHOD1(object_count, size_t(byte* ptr));
    MOCK_CONST_METHOD1(get_type_meta, const gc_type_meta*(byte* ptr));

    MOCK_METHOD1(commit, void(byte* ptr));
    MOCK_METHOD2(commit, void(byte* ptr, const gc_type_meta*));

    MOCK_CONST_METHOD2(trace, void(byte*, const gc_trace_callback&));
    MOCK_METHOD3(move, void(byte*, byte*, memory_descriptor*));
    MOCK_METHOD1(finalize, void(byte*));
};


#endif //DIPLOMA_MEMORY_DESCRIPTOR_MOCK_H
