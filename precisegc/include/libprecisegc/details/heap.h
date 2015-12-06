#ifndef DIPLOMA_HEAP_H
#define DIPLOMA_HEAP_H

#include "segregated_list.h"
#include "mutex.h"
#include "constants.h"

class test1 {};

namespace precisegc { namespace details {

class test {};

class heap
{
public:

    inline static heap& instance()
    {
        static heap h;
        return h;
    }

    void* allocate(size_t size);
    forwarding_list compact();

private:
    static const size_t MIN_ALLOC_SIZE_BITS = 4;
    static const size_t SEGREGATED_STORAGE_SIZE = (SYSTEM_POINTER_BITS_COUNT - RESERVED_BITS_COUNT - 1);

    static size_t align_size(size_t size);

    heap();
    heap(const heap&) = delete;
    heap(const heap&&) = delete;

    heap& operator=(const heap&) = delete;
    heap& operator=(const heap&&) = delete;

    segregated_list m_storage[SEGREGATED_STORAGE_SIZE];
    mutex m_mutex;
};

}}

#endif //DIPLOMA_HEAP_H
