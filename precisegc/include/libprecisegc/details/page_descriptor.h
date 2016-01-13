#ifndef DIPLOMA_PAGE_DESCRIPTOR_H
#define DIPLOMA_PAGE_DESCRIPTOR_H

#include <climits>
#include <bitset>
#include <iterator>
#include <utility>

#include "pool_chunk.h"
#include "constants.h"
#include "iterator_base.h"
#include "iterator_access.h"

namespace precisegc { namespace details {

const size_t OBJECTS_PER_PAGE_BITS = 5;
const size_t OBJECTS_PER_PAGE = (1 << OBJECTS_PER_PAGE_BITS);

const size_t MAX_PAGE_SIZE_BITS = 7;
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
    // TO DO: get rid of it
    void* page() const noexcept;

    void* allocate() noexcept;
    void deallocate(void* ptr) noexcept;

    bool get_object_mark(void* ptr) const noexcept;
    void set_object_mark(void* ptr, bool mark) noexcept;

    bool get_object_pin(void* ptr) const noexcept;
    void set_object_pin(void* ptr, bool pin) noexcept;

    void clear_mark_bits() noexcept;
    void clear_pin_bits() noexcept;

    bool is_memory_available() const noexcept;
    bool is_initialized() const noexcept;

    void* get_object_start(void* ptr) const noexcept;

    iterator begin() noexcept;
    iterator end() noexcept;

    // iterator for iterating through objects in page;
    // we choose bidirectional_iterator_tag concept just because there is no need in more powerful concept;
    // although, iterator for objects in page_descriptor should be random_access;
    class iterator: public iterator_base<iterator, std::bidirectional_iterator_tag, void* const>
    {
    public:
        iterator();
        iterator(const iterator&) noexcept = default;
        iterator(iterator&&) noexcept = default;

        iterator& operator=(const iterator&) noexcept = default;
        iterator& operator=(iterator&&) noexcept = default;

        void* const operator*() const noexcept;

        // WARNING! this methods invalidate iterator
        void set_deallocated() noexcept;

        bool is_marked() const noexcept;
        bool is_pinned() const noexcept;

        void set_marked(bool marked) noexcept;
        void set_pinned(bool pinned) noexcept;

        friend class page_descriptor;
        friend class iterator_access<iterator>;

        std::string tmp() const {
            return m_pd->m_alloc_bits.to_string();
        }

    private:
        iterator(page_descriptor* pd, void* ptr) noexcept;
        size_t get_offset() const noexcept;

        bool equal(const iterator& other) const noexcept;
        void increment() noexcept;
        void decrement() noexcept;

        page_descriptor* m_pd;
        void* m_ptr;
    };

private:
    void index_page();
    void remove_index();

    size_t calculate_offset(void* ptr) const noexcept;

    static std::pair<void*, size_t> allocate_page(size_t obj_size);
    static void deallocate_page(void* page);
    static size_t calculate_mask(size_t page_size, size_t obj_size, void* page_ptr);

    typedef std::bitset<OBJECTS_PER_PAGE> page_bitset;

    size_t m_page_size;
    size_t m_obj_size;
    void* m_page; // pointer on the page itself
    size_t m_mask; // a mask for pointers that points on this page (is used to find object begin)
    page_bitset m_mark_bits; // mark bits for objects in
    page_bitset m_pin_bits; // pin bits for objects in
    page_bitset m_alloc_bits;
    pool_chunk m_pool; // pool structure on page memory
};

} }

#endif // DIPLOMA_PAGE_DESCRIPTOR_H
