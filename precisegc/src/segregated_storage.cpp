#include "segregated_storage.h"

#include <new>

#include "os.h"

namespace precisegc { namespace details {

segregated_list_element::segregated_list_element(segregated_list_element *next, segregated_list_element *prev)
{
    m_header = {next, prev, NULL_PAGE_ID};
}

void* segregated_list_element::allocate(size_t size)
{
    assert(is_memory_available(size));
    if (last_used_page() == NULL_PAGE_ID || !m_pages[last_used_page()].is_memory_available(size)) {

    }
}

size_t segregated_list_element::last_used_page() const noexcept
{
    return m_header.m_last_used_page;
}

void segregated_list_element::set_last_used_page(size_t id) noexcept
{
    m_header.m_last_used_page = id;
}

bool segregated_list_element::is_memory_available(size_t size)
{
    if (last_used_page() == LAST_PAGE_ID) {
        return m_pages[LAST_PAGE_ID].is_memory_available(size);
    }
    return true;
}

segregated_list_element* segregated_list_element::get_next() const noexcept
{
    return m_header.m_next;
}

void segregated_list_element::set_next(segregated_list_element *next) noexcept
{
    m_header.m_next = next;
}

segregated_list_element* segregated_list_element::get_prev() const noexcept
{
    return m_header.m_prev;
}

void segregated_list_element::set_prev(segregated_list_element *prev) noexcept
{
    m_header.m_prev = prev;
}

segregated_list::segregated_list(size_t alloc_size)
    : m_alloc_size(alloc_size)
    , m_first(nullptr)
    , m_last(nullptr)
{}

void* segregated_list::allocate()
{
    if (!m_first) {
        m_first = create_element();
        m_last = m_first;
    }
    if (!m_first->is_memory_available(m_alloc_size)) {
        segregated_list_element* new_sle = create_element(m_first);
        m_first->set_prev(new_sle);
        m_first = new_sle;
    }
    return m_first->allocate(m_alloc_size);
}

segregated_list_element* segregated_list::create_element(segregated_list_element* next, segregated_list_element* prev)
{
    void* mem = memory_allocate(SEGREGATED_STORAGE_ELEMENT_SIZE);
    return new (mem) segregated_list_element(next, prev);
}

}}