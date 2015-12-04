#ifndef DIPLOMA_HEAP_H
#define DIPLOMA_HEAP_H

#include <climits>
#include <bitset>
#include <iterator>
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

    class iterator;

    page_descriptor();
    ~page_descriptor();

    void initialize_page(size_t obj_size);
    void clear_page();

    size_t obj_size() const noexcept;
    size_t page_size() const noexcept;
    void* allocate();

    // clear all memory in range [it, end)
    void clear(const iterator& it);

    void clear_mark_bits() noexcept;
    void clear_pin_bits() noexcept;

    bool is_memory_available() const noexcept;
    bool is_initialized() const noexcept;

    void* get_object_start(void* ptr) const noexcept;

    iterator begin() noexcept;
    iterator end() noexcept;

    // iterator for iterating through objects in page;
    // we choose forward_iterator concept just because there is no need in more powerful concept;
    // although, iterator for objects in page_descriptor should be random_access;
    class iterator: public std::iterator<std::forward_iterator_tag, void* const>
    {
    public:
        iterator();
        iterator(const iterator&) noexcept = default;
        iterator(iterator&&) noexcept = default;

        iterator& operator=(const iterator&) noexcept = default;
        iterator& operator=(iterator&&) noexcept = default;

        void* const operator*() const noexcept;

        iterator operator++() noexcept;
        iterator operator++(int) noexcept;

        iterator operator--() noexcept;
        iterator operator--(int) noexcept;

        bool is_marked() const noexcept;
        bool is_pinned() const noexcept;

        void set_marked(bool marked) noexcept;
        void set_pinned(bool pinned) noexcept;

        friend class page_descriptor;
        friend bool operator==(const page_descriptor::iterator& it1, const page_descriptor::iterator& it2);
    private:
        iterator(page_descriptor* pd, void* ptr) noexcept;

        page_descriptor* m_pd;
        void* m_ptr;
    };

private:
    static std::pair<void*, size_t> allocate_page(size_t obj_size);
    static size_t calculate_mask(size_t page_size, size_t obj_size, void* page_ptr);

    typedef std::bitset<OBJECTS_PER_PAGE> page_bitset;

    size_t m_page_size;
    size_t m_obj_size;
    void* m_page; // pointer on the page itself
    void* m_free; // pointer on the next after the last allocated Object. If Page is full --- NULL
    size_t m_mask; // a mask for pointers that points on this page (is used to find object begin)
    page_bitset m_mark_bits; //mark bits for objects in
    page_bitset m_pin_bits; //pin bits for objects in
};

bool operator==(const page_descriptor::iterator& it1, const page_descriptor::iterator& it2);
bool operator!=(const page_descriptor::iterator& it1, const page_descriptor::iterator& it2);

} }

#endif //DIPLOMA_HEAP_H
