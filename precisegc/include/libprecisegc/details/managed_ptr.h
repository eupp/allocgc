#ifndef DIPLOMA_MANAGED_PTR_H
#define DIPLOMA_MANAGED_PTR_H

#include <memory>
#include <mutex>
#include <exception>

#include "allocators/index_tree.h"
#include "allocators/indexed_ptr.h"
#include "allocators/pointer_decorator.h"
#include "managed_memory_descriptor.h"
#include "util.h"

namespace precisegc { namespace details {

typedef allocators::index_tree<managed_memory_descriptor, std::allocator<byte>> index_type;
typedef allocators::indexed_ptr<byte, index_type> managed_ptr;

class managed_cell_ptr : public allocators::pointer_decorator<managed_cell_ptr, managed_ptr>
{
    typedef allocators::pointer_decorator<managed_cell_ptr, managed_ptr> pointer_decorator_t;
public:
    typedef managed_memory_descriptor::lock_type lock_type;

    managed_cell_ptr();
    managed_cell_ptr(nullptr_t);
    managed_cell_ptr(managed_ptr idx_ptr);
    managed_cell_ptr(managed_ptr idx_ptr, managed_memory_descriptor* descriptor, lock_type&& lock);

    managed_cell_ptr(const managed_cell_ptr& other);
    managed_cell_ptr& operator=(const managed_cell_ptr& other);

    managed_cell_ptr(managed_cell_ptr&& other) = default;
    managed_cell_ptr& operator=(managed_cell_ptr&& other) = default;

    bool get_mark() const;
    bool get_pin() const;

    void set_mark(byte* ptr, bool mark);
    void set_pin(byte* ptr, bool pin);

    void shade(byte* ptr);

    object_meta* get_meta() const;

    bool owns_descriptor_lock() const;
private:
    managed_memory_descriptor* m_descr;
    lock_type m_lock;
};

}}

#endif //DIPLOMA_MANAGED_PTR_H
