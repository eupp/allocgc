#ifndef DIPLOMA_HEAP_H
#define DIPLOMA_HEAP_H

#include <climits>

#include "managed_object.h"

namespace precisegc { namespace details {

const size_t SYSTEM_POINTER_SIZE = 64;
const size_t RESERVED_BITS_COUNT = 3;

const size_t CELL_BITS = 12;
const size_t MEMORY_CELL_SIZE = 1 << CELL_BITS;

const size_t OBJECTS_PER_PAGE_BITS = 5;
const size_t OBJECTS_PER_PAGE = (1 << OBJECTS_PER_PAGE_BITS);

const size_t SEGREGATED_STORAGE_SIZE = SYSTEM_POINTER_SIZE - RESERVED_BITS_COUNT - 1;
const size_t SEGREGATED_STORAGE_ELEMENT_SIZE = 4096;

const size_t BITS_PER_LONG = sizeof(long) * CHAR_BIT;
const size_t MARK_BIT_ARRAY_SIZE = OBJECTS_PER_PAGE / BITS_PER_LONG + 1;

class bit_array {
public:

    bit_array();

    bool get(size_t i);
    void set(size_t i, bool value);

private:
    long m_bits[MARK_BIT_ARRAY_SIZE];
};

class page_descriptor {
public:

    managed_object* allocate();
    bool is_full() const noexcept;

private:
    size_t m_obj_size; // sizeof object (whole object (ex. in array of case --- whole array) with a header)
    size_t m_page_size;
    size_t m_mask; // a mask for pointers that points on this page (is used to find object begin)
    void* m_free; // pointer on the next after the last allocated Object. If Page is full --- NULL
    void* m_page; // pointer on the page itself
    bit_array m_mark_bits; //mark bits for objects in
    bit_array m_pin_bits; //pin bits for objects in
};


} }

#endif //DIPLOMA_HEAP_H
