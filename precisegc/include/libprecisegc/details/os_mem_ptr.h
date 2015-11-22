#ifndef DIPLOMA_MMAP_PTR_H
#define DIPLOMA_MMAP_PTR_H

#include <memory>

#include "os.h"

namespace precisegc { namespace details {

template <typename T>
class os_mem_deleter
{
public:

    os_mem_deleter(size_t size = sizeof(T))
        : m_size(size)
    {}

    void operator()(T* ptr) const
    {
        memory_deallocate(ptr, m_size);
    }
private:
    size_t m_size;
};

template <typename T>
using os_mem_unique_ptr = std::unique_ptr<T, os_mem_deleter<T>>;

}}

#endif //DIPLOMA_MMAP_PTR_H
