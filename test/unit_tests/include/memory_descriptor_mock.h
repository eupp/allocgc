#ifndef DIPLOMA_MEMORY_DESCRIPTOR_MOCK_H
#define DIPLOMA_MEMORY_DESCRIPTOR_MOCK_H

#include <gmock/gmock.h>

#include "libprecisegc/details/managed_pool_chunk.h"

class memory_descriptor_mock : public precisegc::details::managed_memory_descriptor
{
    typedef precisegc::details::byte byte;
public:
    MOCK_METHOD1(get_mark, bool(byte*));
    MOCK_METHOD1(get_pin, bool(byte*));
    MOCK_METHOD2(set_mark, void(byte* ptr, bool mark));
    MOCK_METHOD2(set_pin, void(byte* ptr, bool pin));

    MOCK_METHOD1(sweep, void (byte* ptr));

    MOCK_METHOD1(is_live, bool (byte* ptr));

    MOCK_METHOD1(get_cell_meta, precisegc::details::object_meta* (byte* ptr));
    MOCK_METHOD1(get_object_begin, byte*(byte* ptr));
    MOCK_METHOD1(get_cell_begin, byte*(byte* ptr));

    MOCK_METHOD0(lock, lock_type());
    MOCK_METHOD1(lock, lock_type(std::defer_lock_t));
    MOCK_METHOD1(lock, lock_type(std::try_to_lock_t));
    MOCK_METHOD1(lock, lock_type(std::adopt_lock_t));
};


#endif //DIPLOMA_MEMORY_DESCRIPTOR_MOCK_H
