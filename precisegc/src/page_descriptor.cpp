#include "page_descriptor.h"

#include <cassert>

#include "os.h"
#include "util.h"

namespace precisegc { namespace details {

page_descriptor::page_descriptor()
    : m_page(nullptr)
    , m_free(nullptr)
    , m_page_size(0)
    , m_obj_size(0)
    , m_mask(0)
{ }

page_descriptor::~page_descriptor()
{
    clear_page();
}

void page_descriptor::initialize_page(size_t obj_size)
{
    clear_page();
    auto alloc_res = allocate_page(obj_size);
    m_page = alloc_res.first;
    m_page_size = alloc_res.second;
    m_obj_size = obj_size;
    m_free = m_page;
    m_mask = calculate_mask(m_page_size, obj_size, m_page);
}

void page_descriptor::clear_page()
{
    if (m_page) {
        memory_align_deallocate(m_page);
        m_page = nullptr;
        m_free = nullptr;
        m_page_size = 0;
        m_obj_size = 0;
        m_mask = 0;
        m_mark_bits.reset();
        m_pin_bits.reset();
    }
}

void* page_descriptor::allocate()
{
    assert(is_memory_available());
    void* res = m_free;
    m_free = (void*) ((size_t) m_free + m_obj_size);
    return res;
}

size_t page_descriptor::obj_size() const noexcept
{
    return m_obj_size;
}

size_t page_descriptor::page_size() const noexcept
{
    return m_page_size;
}

bool page_descriptor::is_memory_available() const noexcept
{
    return m_page && ((size_t) m_free <= (size_t) m_page + m_page_size - m_obj_size);
}

bool page_descriptor::is_initialized() const noexcept
{
    return m_page != nullptr;
}

void* page_descriptor::get_object_start(void *ptr) const noexcept
{
    size_t ptr_ = (size_t) ptr;
    assert(((size_t) m_page <= ptr_) && (ptr_ < (size_t) m_page + m_page_size));
    return (void*) (ptr_ & m_mask);
}

std::pair<void*, size_t> page_descriptor::allocate_page(size_t obj_size)
{
    void* page = nullptr;
    size_t obj_count_bits = OBJECTS_PER_PAGE_BITS;
    size_t page_size = obj_size << obj_count_bits;
    while (!page) {
        assert(obj_count_bits <= MAX_PAGE_SIZE_BITS);
        page = memory_align_allocate(page_size, page_size);
        page_size <<= 1;
        obj_count_bits += 1;
    }
    assert(page);
    return std::make_pair(page, page_size >> 1);
}

page_descriptor::iterator page_descriptor::begin() const noexcept
{
    return iterator(this, m_page);
}

page_descriptor::iterator page_descriptor::end() const noexcept
{
    return iterator(this, m_free);
}

size_t page_descriptor::calculate_mask(size_t page_size, size_t obj_size, void* page_ptr)
{
    int page_count_bits = log_2(page_size);
    int obj_size_bits = log_2(obj_size);
    int bit_diff = page_count_bits - obj_size_bits;
    assert(page_count_bits != -1 && obj_size_bits != -1);
    return ((size_t)page_ptr | ((1 << bit_diff) - 1) << obj_size_bits);
}

page_descriptor::iterator::iterator()
    : m_pd(nullptr)
    , m_ptr(nullptr)
{}


page_descriptor::iterator::iterator(const page_descriptor *pd, void *ptr) noexcept
    : m_pd(pd)
    , m_ptr(ptr)
{}

void* const page_descriptor::iterator::operator*() const noexcept
{
    assert(m_ptr);
    assert(*this != m_pd->end());
    return m_ptr;
}

page_descriptor::iterator page_descriptor::iterator::operator++() noexcept
{
    assert(m_pd->is_initialized());
    assert((size_t) m_ptr < (size_t) m_pd->m_page + m_pd->m_page_size);
    m_ptr = (void*) ((size_t) m_ptr + m_pd->m_obj_size);
    return *this;
}

page_descriptor::iterator page_descriptor::iterator::operator++(int) noexcept
{
    iterator it = *this;
    ++(*this);
    return it;
}

page_descriptor::iterator page_descriptor::iterator::operator--() noexcept
{
    assert(m_pd->is_initialized());
    assert(m_ptr > m_pd->m_page);
    m_ptr = (void*) ((size_t) m_ptr - m_pd->m_obj_size);
    return *this;
}

page_descriptor::iterator page_descriptor::iterator::operator--(int) noexcept
{
    iterator it = *this;
    --(*this);
    return it;
}

bool operator==(const page_descriptor::iterator& it1, const page_descriptor::iterator& it2)
{
    return it1.m_ptr == it2.m_ptr;
}

bool operator!=(const page_descriptor::iterator& it1, const page_descriptor::iterator& it2)
{
    return !(it1 == it2);
}

bool page_descriptor::iterator::is_marked() const noexcept
{
    assert(m_ptr);
    assert(*this != m_pd->end());
    size_t offset = (size_t) m_ptr - (size_t) m_pd->m_page;
    return m_pd->m_mark_bits[offset];
}

bool page_descriptor::iterator::is_pinned() const noexcept
{
    assert(m_ptr);
    assert(*this != m_pd->end());
    size_t offset = (size_t) m_ptr - (size_t) m_pd->m_page;
    return m_pd->m_pin_bits[offset];
}

}}