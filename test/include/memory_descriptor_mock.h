#ifndef DIPLOMA_MEMORY_DESCRIPTOR_MOCK_H
#define DIPLOMA_MEMORY_DESCRIPTOR_MOCK_H

#include <gmock/gmock.h>

#include "libprecisegc/details/allocators/managed_pool_chunk.hpp"

class memory_descriptor_mock : public precisegc::details::memory_descriptor
{
    typedef precisegc::details::byte byte;
    typedef precisegc::details::gc_type_meta type_meta;
    typedef precisegc::details::collectors::traceable_object_meta traceable_object_meta;
public:
    MOCK_CONST_METHOD1(get_mark, bool(byte*));
    MOCK_CONST_METHOD1(get_pin, bool(byte*));

    MOCK_METHOD2(set_mark, void(byte* ptr, bool mark));
    MOCK_METHOD2(set_pin, void(byte* ptr, bool pin));

    size_t cell_size(byte* ptr);
    MOCK_CONST_METHOD1(cell_start, byte*(byte* ptr));

    MOCK_CONST_METHOD1(get_type_meta, const type_meta*(byte* ptr));
    MOCK_METHOD2(set_type_meta, void(byte* ptr, const type_meta*));
};


#endif //DIPLOMA_MEMORY_DESCRIPTOR_MOCK_H
