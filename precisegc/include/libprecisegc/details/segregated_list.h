#ifndef DIPLOMA_SEGREGATED_STORAGE_H
#define DIPLOMA_SEGREGATED_STORAGE_H

#include <cstddef>
#include <utility>

#include "constants.h"
#include "page_descriptor.h"
#include "forwarding_list.h"
#include "iterator_base.h"
#include "iterator_access.h"

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
    bool is_memory_available() const noexcept;

    // clear all memory in range [ pages[page_id]->it, end)
    void clear(const page_descriptor::iterator& it, size_t page_id);

    void clear_mark_bits() noexcept;
    void clear_pin_bits() noexcept;

    page_descriptor& get_page_descriptor(size_t ind);

    segregated_list_element* get_next() const noexcept;
    void set_next(segregated_list_element* next) noexcept;

    segregated_list_element* get_prev() const noexcept;
    void set_prev(segregated_list_element* prev) noexcept;

    size_t last_used_page() const noexcept;
    void set_last_used_page(size_t id) noexcept;

private:
    segregated_list_element_header m_header;
    page_descriptor m_pages[PAGES_PER_SEGREGATED_STORAGE_ELEMENT];
};

class segregated_list
{
public:

    class iterator;

    segregated_list();
    segregated_list(size_t alloc_size);
    ~segregated_list();

    allocate_result allocate();
    void compact(forwarding_list& forwarding);
    void fix_pointers(const forwarding_list& forwarding);

    // clear all memory in range [it, end)
    void clear(const iterator& it);

    iterator begin() noexcept;
    iterator end() noexcept;

    // design flaw: it may be called one time before initialization (i.e. after default constructor)
    void set_alloc_size(size_t size) noexcept;
    size_t alloc_size() const noexcept;

    // iterator for iterating through objects in segregated_list;
    // we choose bidirectional_iterator_tag concept just because there is no need in more powerful concept;
    // although, iterator for objects in segregated_list should be random_access;
    class iterator: public iterator_base<iterator, std::bidirectional_iterator_tag, void* const>
    {
    public:
        iterator(const iterator&) noexcept = default;
        iterator(iterator&&) noexcept = default;

        iterator& operator=(const iterator&) noexcept = default;
        iterator& operator=(iterator&&) noexcept = default;

        void* const operator*() const noexcept;

        bool is_marked() const noexcept;
        bool is_pinned() const noexcept;

        void set_marked(bool marked) noexcept;
        void set_pinned(bool pinned) noexcept;

        friend class segregated_list;
        friend class iterator_access<iterator>;
    private:
        iterator(segregated_list_element* sle, size_t pd_ind, page_descriptor::iterator pd_itr) noexcept;

        bool equal(const iterator& other) const noexcept;
        void increment() noexcept;
        void decrement() noexcept;

        segregated_list_element* m_sle;
        size_t m_pd_ind;
        page_descriptor::iterator m_pd_itr;
    };

private:
    void fix_ptr(void* ptr, const forwarding_list& forwarding);
    void clear_mark_bits() noexcept;
    void clear_pin_bits() noexcept;

    size_t m_alloc_size;
    segregated_list_element* m_first;
    segregated_list_element* m_last;
};

bool operator==(const segregated_list::iterator& it1, const segregated_list::iterator& it2);
bool operator!=(const segregated_list::iterator& it1, const segregated_list::iterator& it2);

}}

#endif //DIPLOMA_SEGREGATED_STORAGE_H
