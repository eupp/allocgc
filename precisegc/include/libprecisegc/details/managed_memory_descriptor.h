#ifndef DIPLOMA_CELL_DESCRIPTOR_H
#define DIPLOMA_CELL_DESCRIPTOR_H

#include <mutex>

#include "allocators/types.h"
#include "object_meta.h"
#include "mutex.h"

namespace precisegc { namespace details {

class managed_memory_descriptor
{
public:
    typedef std::unique_lock<mutex> lock_type;

    virtual ~managed_memory_descriptor() {}

    virtual bool get_mark(byte* ptr) = 0;
    virtual bool get_pin(byte* ptr) = 0;

    virtual void set_mark(byte* ptr, bool mark) = 0;
    virtual void set_pin(byte* ptr, bool pin) = 0;

//    virtual void shade(byte* ptr) = 0;

    virtual object_meta* get_cell_meta(byte* ptr) = 0;
    virtual byte* get_object_begin(byte* ptr) = 0;

    virtual lock_type lock() = 0;
    virtual lock_type lock(std::defer_lock_t) = 0;
    virtual lock_type lock(std::try_to_lock_t) = 0;
    virtual lock_type lock(std::adopt_lock_t) = 0;
};

}}

#endif //DIPLOMA_CELL_DESCRIPTOR_H