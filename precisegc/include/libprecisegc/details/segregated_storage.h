#ifndef DIPLOMA_SEGREGATED_STORAGE_H
#define DIPLOMA_SEGREGATED_STORAGE_H

#include <cstddef>

#include "constants.h"
#include "page_descriptor.h"

namespace precisegc { namespace details {

const size_t SEGREGATED_STORAGE_BITS_SIZE = SYSTEM_POINTER_BITS_COUNT - RESERVED_BITS_COUNT - 1;
const size_t SEGREGATED_STORAGE_ELEMENT_SIZE = 4096;

class segregated_list_element;

struct segregated_list_element_header
{
    segregated_list_element* m_prev;
    segregated_list_element* m_next;
    size_t m_last_used_page;
};

const size_t PAGES_PER_SEGREGATED_STORAGE_ELEMENT =
        (SEGREGATED_STORAGE_ELEMENT_SIZE - sizeof(segregated_list_element_header)) / sizeof(page_descriptor);

const size_t LAST_PAGE_ID = PAGES_PER_SEGREGATED_STORAGE_ELEMENT - 1;
// NULL_PAGE_ID - id of page in segregated_list_element that should't be used to allocate memory,
//                is is used in initialization of list element (as last_used_page) but before any allocation
const size_t NULL_PAGE_ID = PAGES_PER_SEGREGATED_STORAGE_ELEMENT;

class segregated_list_element
{
public:

    segregated_list_element(segregated_list_element* next, segregated_list_element* prev);

    void* allocate(size_t size);
    bool is_memory_available(size_t size);

    segregated_list_element* get_next() const noexcept;
    void set_next(segregated_list_element* next) noexcept;

    segregated_list_element* get_prev() const noexcept;
    void set_prev(segregated_list_element* prev) noexcept;

private:

    size_t last_used_page() const noexcept;
    void set_last_used_page(size_t id) noexcept;

    segregated_list_element_header m_header;
    page_descriptor m_pages[PAGES_PER_SEGREGATED_STORAGE_ELEMENT];
};

class segregated_list
{
public:

    segregated_list(size_t alloc_size);
    ~segregated_list();

    void* allocate();

private:
    static segregated_list_element* create_element(segregated_list_element* next = nullptr,
                                                   segregated_list_element* prev = nullptr);

    size_t m_alloc_size;
    segregated_list_element* m_first;
    segregated_list_element* m_last;
};

}}

#endif //DIPLOMA_SEGREGATED_STORAGE_H
