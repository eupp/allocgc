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

class ptr_manager : private noncopyable
{
    typedef typename managed_memory_descriptor::lock_type lock_type;
public:
    ptr_manager();
    ptr_manager(nullptr_t);
    ptr_manager(managed_ptr idx_ptr);
    ptr_manager(managed_ptr idx_ptr, managed_memory_descriptor* descriptor, lock_type&& lock);

    ptr_manager(ptr_manager&& other);
    ptr_manager& operator=(ptr_manager&& other);

    bool get_mark() const;
    bool get_pin() const;

    void set_mark(byte* ptr, bool mark);
    void set_pin(byte* ptr, bool pin);

    void shade(byte* ptr);

    object_meta* get_meta() const;

    byte* get() const;
private:
    managed_ptr m_ptr;
    managed_memory_descriptor* m_descriptor;
    lock_type m_lock;
};

}}

#endif //DIPLOMA_MANAGED_PTR_H
