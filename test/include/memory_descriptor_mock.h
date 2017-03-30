#ifndef ALLOCGC_MEMORY_DESCRIPTOR_MOCK_H
#define ALLOCGC_MEMORY_DESCRIPTOR_MOCK_H

#include <gmock/gmock.h>

#include "liballocgc/details/allocators/gc_pool_descriptor.hpp"

class memory_descriptor_mock : public allocgc::details::allocators::gc_memory_descriptor
{
    typedef allocgc::byte byte;
    typedef allocgc::gc_type_meta gc_type_meta;
    typedef allocgc::details::gc_lifetime_tag gc_lifetime_tag;
    typedef allocgc::details::gc_trace_callback gc_trace_callback;
    typedef allocgc::details::allocators::gc_memory_descriptor memory_descriptor;
public:
    MOCK_CONST_METHOD1(get_mark, bool(byte*));
    MOCK_CONST_METHOD1(get_pin, bool(byte*));

    MOCK_METHOD2(set_mark, void(byte* ptr, bool mark));
    MOCK_METHOD2(set_pin, void(byte* ptr, bool pin));

    MOCK_CONST_METHOD1(is_init, bool(byte*));

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


#endif //ALLOCGC_MEMORY_DESCRIPTOR_MOCK_H
