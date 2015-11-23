#include "page_descriptor.h"

#include <cassert>

#include "os.h"
#include "util.h"

namespace precisegc { namespace details {

page_descriptor::page_descriptor()
    : m_page(nullptr)
    , m_free(nullptr)
    , m_page_size(0)
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
        m_mask = 0;
        m_mark_bits.reset();
        m_pin_bits.reset();
    }
}

void* page_descriptor::allocate(size_t obj_size)
{
    assert(is_memory_available(obj_size));
    void* res = m_free;
    m_free = (void*) ((size_t) m_free + obj_size);
    return res;
}

bool page_descriptor::is_memory_available(size_t size) const noexcept
{
    return (size <= m_page_size) && ((size_t) m_free <= (size_t) m_page + m_page_size - size);
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

size_t page_descriptor::calculate_mask(size_t page_size, size_t obj_size, void* page_ptr)
{
    int page_count_bits = log_2(page_size);
    int obj_size_bits = log_2(obj_size);
    int bit_diff = page_count_bits - obj_size_bits;
    assert(page_count_bits != -1 && obj_size_bits != -1);
    return ((size_t)page_ptr | ((1 << bit_diff) - 1) << obj_size_bits);
}

}}