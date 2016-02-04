#include "page_descriptor.h"

#include <cassert>

#include "os.h"
#include "util.h"
#include "index_tree.h"

namespace precisegc { namespace details {

page_descriptor::page_descriptor()
    : m_page(nullptr)
    , m_pool(nullptr, 0, 0)
    , m_page_size(0)
    , m_obj_size(0)
    , m_mask(0)
{
    m_alloc_bits.set();
}

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
    m_pool.reset(m_page, m_obj_size, m_page_size / m_obj_size);
    m_alloc_bits.reset();
    m_mask = calculate_mask(m_page_size, obj_size, m_page);
    index_page();
}

void page_descriptor::clear_page()
{
    if (m_page) {
        remove_index();
        deallocate_page(m_page);
        m_page = nullptr;
        m_pool.reset(nullptr, 0, 0);
        m_page_size = 0;
        m_obj_size = 0;
        m_mask = 0;
        m_mark_bits.reset();
        m_pin_bits.reset();
        m_alloc_bits.set();
    }
}

void* page_descriptor::allocate() noexcept
{
    assert(is_memory_available());
    void* ptr = m_pool.allocate(m_obj_size);
    m_alloc_bits[calculate_offset(ptr)] = true;
    return ptr;
}

void page_descriptor::deallocate(void* ptr) noexcept
{
    m_alloc_bits[calculate_offset(ptr)] = false;
    m_pool.deallocate(ptr, m_obj_size);
}

void page_descriptor::clear_mark_bits() noexcept
{
    m_mark_bits.reset();
}

void page_descriptor::clear_pin_bits() noexcept
{
    m_pin_bits.reset();
}

bool page_descriptor::get_object_mark(void* ptr) const noexcept
{
    size_t ind = calculate_offset(get_object_start(ptr));
    return m_mark_bits[ind];
}

void page_descriptor::set_object_mark(void* ptr, bool mark) noexcept
{
    size_t ind = calculate_offset(get_object_start(ptr));
    m_mark_bits[ind] = mark;
}

bool page_descriptor::get_object_pin(void* ptr) const noexcept
{
    size_t ind = calculate_offset(get_object_start(ptr));
    return m_pin_bits[ind];
}

void page_descriptor::set_object_pin(void* ptr, bool pin) noexcept
{
    size_t ind = calculate_offset(get_object_start(ptr));
    m_pin_bits[ind] = pin;
}

size_t page_descriptor::obj_size() const noexcept
{
    return m_obj_size;
}

size_t page_descriptor::page_size() const noexcept
{
    return m_page_size;
}

void* page_descriptor::page() const noexcept
{
    return m_page;
}

bool page_descriptor::is_memory_available() const noexcept
{
    return !m_alloc_bits.all();
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

void page_descriptor::index_page()
{
    size_t page_end = (size_t) m_page + m_page_size;
    for (size_t it = (size_t) m_page; it < page_end; it += MEMORY_CELL_SIZE) {
        assert(((size_t)it & ((size_t)1 << MEMORY_CELL_SIZE_BITS) - 1) == 0);
        _GC_::IT_index_new_cell((void*) it, this);
    }
}

void page_descriptor::remove_index()
{
    size_t page_end = (size_t) m_page + m_page_size;
    for (size_t it = (size_t) m_page; it < page_end; it += MEMORY_CELL_SIZE) {
        _GC_::IT_remove_index((void*) it);
    }
}

std::pair<void*, size_t> page_descriptor::allocate_page(size_t obj_size)
{
    void* page = nullptr;
    size_t obj_count_bits = MAX_OBJECTS_PER_PAGE_BITS;
    // 128 * obj_size
    size_t size = obj_size << obj_count_bits;
    size_t mem_cell_cnt = (size + MEMORY_CELL_SIZE - 1) / MEMORY_CELL_SIZE;
    size_t page_size = mem_cell_cnt * MEMORY_CELL_SIZE;
    page = memory_align_allocate(page_size, page_size);
    assert(page);
    return std::make_pair(page, page_size);
}

void page_descriptor::deallocate_page(void* page)
{
    memory_align_deallocate(page);
}

page_descriptor::iterator page_descriptor::begin() noexcept
{
    void* ptr = m_page;
    size_t ind = 0;
    while (((size_t) ptr < (size_t) m_page + m_page_size) && !m_alloc_bits[ind]) {
        ptr = (void*) ((size_t) ptr + m_obj_size);
        ind++;
    }
    return iterator(this, ptr);
}

page_descriptor::iterator page_descriptor::end() noexcept
{
    return iterator(this, (void*) ((size_t) m_page + m_page_size));
}

size_t page_descriptor::calculate_mask(size_t page_size, size_t obj_size, void* page_ptr)
{
    size_t page_count_bits = log_2(page_size);
    size_t obj_size_bits = log_2(obj_size);
    size_t bit_diff = page_count_bits - obj_size_bits;
    return ((size_t)page_ptr | ((1 << bit_diff) - 1) << obj_size_bits);
}

size_t page_descriptor::calculate_offset(void* ptr) const noexcept
{
    assert(m_page);
    assert(m_page <= ptr && ptr <= (void*) ((size_t) m_page + m_page_size));
    return ((size_t) ptr - (size_t) m_page) / m_obj_size;
}


page_descriptor::iterator::iterator()
    : m_pd(nullptr)
    , m_ptr(nullptr)
{}

page_descriptor::iterator::iterator(page_descriptor *pd, void *ptr) noexcept
    : m_pd(pd)
    , m_ptr(ptr)
{}

void* const page_descriptor::iterator::operator*() const noexcept
{
    assert(m_ptr);
    assert(*this != m_pd->end());
    return m_ptr;
}

void page_descriptor::iterator::increment() noexcept
{
    assert(m_pd->is_initialized());
    assert((size_t) m_ptr < (size_t) m_pd->m_page + m_pd->m_page_size);
    do {
        m_ptr = (void*) ((size_t) m_ptr + m_pd->m_obj_size);
    } while (((size_t) m_ptr < (size_t) m_pd->m_page + m_pd->m_page_size) && !m_pd->m_alloc_bits[get_offset()]);
}

void page_descriptor::iterator::decrement() noexcept
{
    assert(m_pd->is_initialized());
    assert(m_ptr > m_pd->m_page);
    do {
        m_ptr = (void*) ((size_t) m_ptr - m_pd->m_obj_size);
    } while (((size_t) m_ptr > (size_t) m_pd->m_page) && !m_pd->m_alloc_bits[get_offset()]);
}

bool page_descriptor::iterator::equal(const iterator &other) const noexcept
{
    return m_ptr == other.m_ptr;
}

size_t page_descriptor::iterator::get_offset() const noexcept
{
    return m_pd->calculate_offset(m_ptr);
}

void page_descriptor::iterator::set_deallocated() noexcept
{
    m_pd->deallocate(m_ptr);
}

bool page_descriptor::iterator::is_marked() const noexcept
{
    return m_pd->m_mark_bits[get_offset()];
}

bool page_descriptor::iterator::is_pinned() const noexcept
{
    return m_pd->m_pin_bits[get_offset()];
}

void page_descriptor::iterator::set_marked(bool marked) noexcept
{
    m_pd->m_mark_bits[get_offset()] = marked;
}

void page_descriptor::iterator::set_pinned(bool pinned) noexcept
{
    m_pd->m_pin_bits[get_offset()] = pinned;
}

}}