#ifndef DIPLOMA_HEAP_H
#define DIPLOMA_HEAP_H

#include <climits>
#include <bitset>
#include <utility>

#include "constants.h"

namespace precisegc { namespace details {

const size_t MEMORY_CELL_SIZE_BITS = 12;
const size_t MEMORY_CELL_SIZE = 1 << MEMORY_CELL_SIZE_BITS;

const size_t OBJECTS_PER_PAGE_BITS = 5;
const size_t OBJECTS_PER_PAGE = (1 << OBJECTS_PER_PAGE_BITS);

const size_t MAX_PAGE_SIZE_BITS = 15;
const size_t MAX_PAGE_SIZE = 1 << MAX_PAGE_SIZE_BITS;

class page_descriptor
{
public:

    page_descriptor();
    ~page_descriptor();

    void initialize_page(size_t obj_size);

    void* allocate(size_t obj_size);

    bool is_memory_available(size_t size) const noexcept;

    void* get_object_start(void* ptr) const noexcept;

private:
    void clear_page();

    static std::pair<void*, size_t> allocate_page(size_t obj_size);
    static size_t calculate_mask(size_t page_size, size_t obj_size, void* page_ptr);

    typedef std::bitset<OBJECTS_PER_PAGE> page_bitset;

    size_t m_page_size;
    void* m_page; // pointer on the page itself
    void* m_free; // pointer on the next after the last allocated Object. If Page is full --- NULL
    size_t m_mask; // a mask for pointers that points on this page (is used to find object begin)
    page_bitset m_mark_bits; //mark bits for objects in
    page_bitset m_pin_bits; //pin bits for objects in
};


} }

#endif //DIPLOMA_HEAP_H
