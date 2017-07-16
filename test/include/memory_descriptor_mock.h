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
    typedef allocgc::details::allocators::gc_memory_descriptor::box_id box_id;
public:
    MOCK_CONST_METHOD1(get_id, box_id(byte*));

    MOCK_CONST_METHOD1(get_mark, bool(box_id));
    MOCK_CONST_METHOD1(get_pin, bool(box_id));

    MOCK_METHOD2(set_mark, void(box_id, bool mark));
    MOCK_METHOD2(set_pin, void(box_id, bool pin));

    MOCK_CONST_METHOD1(is_init, bool(box_id));

    MOCK_CONST_METHOD1(get_lifetime_tag, gc_lifetime_tag(box_id));

    MOCK_CONST_METHOD1(box_addr, byte*(box_id));
    MOCK_CONST_METHOD1(box_size, size_t(box_id));

    MOCK_CONST_METHOD1(object_count, size_t(box_id));
    MOCK_CONST_METHOD1(get_type_meta, const gc_type_meta*(box_id));

    MOCK_METHOD1(commit, void(box_id));
    MOCK_METHOD2(commit, void(box_id, const gc_type_meta*));

    MOCK_CONST_METHOD2(trace, void(box_id, const gc_trace_callback&));
//    MOCK_METHOD3(move, void(byte*, byte*, memory_descriptor*));
    MOCK_METHOD1(finalize, void(box_id));
};


#endif //ALLOCGC_MEMORY_DESCRIPTOR_MOCK_H
