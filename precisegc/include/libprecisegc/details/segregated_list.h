#ifndef DIPLOMA_SEGREGATED_STORAGE_H
#define DIPLOMA_SEGREGATED_STORAGE_H

#include <cstddef>
#include <utility>

#include "constants.h"
#include "page_descriptor.h"
#include "forwarding_list.h"

namespace precisegc { namespace details {

const size_t SEGREGATED_STORAGE_BITS_SIZE = SYSTEM_POINTER_BITS_COUNT - RESERVED_BITS_COUNT - 1;
const size_t SEGREGATED_STORAGE_ELEMENT_SIZE = 4096;

typedef std::pair<void*, page_descriptor*> allocate_result;

class segregated_list_element;

struct segregated_list_element_header
{
    segregated_list_element* m_next;
    segregated_list_element* m_prev;
    size_t m_last_used_page;
};

const size_t PAGES_PER_SEGREGATED_STORAGE_ELEMENT =
        (SEGREGATED_STORAGE_ELEMENT_SIZE - sizeof(segregated_list_element_header)) / sizeof(page_descriptor);

const size_t LAST_PAGE_ID = PAGES_PER_SEGREGATED_STORAGE_ELEMENT - 1;
// NULL_PAGE_ID - id of page in segregated_list_element that should't be used to allocate memory,
//                is is used in initialization of list element (as last_used_page) but before any allocation
//const size_t NULL_PAGE_ID = PAGES_PER_SEGREGATED_STORAGE_ELEMENT;

class segregated_list_element
{
public:

    static void* operator new(size_t size);
    static void operator delete(void* ptr);

    segregated_list_element(size_t obj_size,
                            segregated_list_element* next = nullptr,
                            segregated_list_element* prev = nullptr);

    allocate_result allocate();
    bool is_memory_available();

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

    allocate_result allocate();
    forwarding_list compact();

private:
    void* get_next_unmarked(segregated_list_element* elem, page_descriptor* begin, page_descriptor* end);
    void* get_next_marked(segregated_list_element* elem, page_descriptor* begin, page_descriptor* end);

    size_t m_alloc_size;
    segregated_list_element* m_first;
    segregated_list_element* m_last;
};

}}

#endif //DIPLOMA_SEGREGATED_STORAGE_H
