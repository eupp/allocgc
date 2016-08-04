#ifndef DIPLOMA_CELL_DESCRIPTOR_H
#define DIPLOMA_CELL_DESCRIPTOR_H

#include <mutex>

#include "types.hpp"
#include "object_meta.hpp"

namespace precisegc { namespace details {

class memory_descriptor
{
public:
    typedef std::recursive_mutex mutex_type;
    typedef std::unique_lock<mutex_type> lock_type;

    virtual ~memory_descriptor() {}

    virtual bool get_mark(byte* ptr) const = 0;
    virtual bool get_pin(byte* ptr) const = 0;

    virtual void set_mark(byte* ptr, bool mark) = 0;
    virtual void set_pin(byte* ptr, bool pin) = 0;

    virtual bool is_live(byte* ptr) const = 0;
    virtual void set_live(byte* ptr, bool live) = 0;

    virtual void sweep(byte* ptr) = 0;

//    virtual void shade(byte* ptr) = 0;

    virtual size_t cell_size() const  = 0;

    virtual object_meta* get_cell_meta(byte* ptr) const = 0;
    virtual byte* get_cell_begin(byte* ptr) const = 0;
    virtual byte* get_obj_begin(byte* ptr) const = 0;
};

}}

#endif //DIPLOMA_CELL_DESCRIPTOR_H
