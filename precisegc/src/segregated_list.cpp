#include "segregated_list.h"

#include <new>

#include "os.h"

namespace precisegc { namespace details {

void* segregated_list_element::operator new(size_t )
{
    return memory_allocate(SEGREGATED_STORAGE_ELEMENT_SIZE);
}

void segregated_list_element::operator delete(void* ptr)
{
    memory_deallocate(ptr, SEGREGATED_STORAGE_ELEMENT_SIZE);
}

segregated_list_element::segregated_list_element(size_t obj_size,
                                                 segregated_list_element *next,
                                                 segregated_list_element *prev)
{
    for (size_t i = 0; i < PAGES_PER_SEGREGATED_STORAGE_ELEMENT; ++i) {
        m_pages[i].initialize_page(obj_size);
    }
    m_header = {next, prev, 0};

}

allocate_result segregated_list_element::allocate()
{
    assert(is_memory_available());
    size_t page_id = last_used_page();
    if (!m_pages[page_id].is_memory_available()) {
        page_id++;
        set_last_used_page(page_id);
    }
    void* mem = m_pages[page_id].allocate();
    return std::make_pair(mem, &m_pages[page_id]);
}

size_t segregated_list_element::last_used_page() const noexcept
{
    return m_header.m_last_used_page;
}

void segregated_list_element::set_last_used_page(size_t id) noexcept
{
    m_header.m_last_used_page = id;
}

bool segregated_list_element::is_memory_available()
{
    if (last_used_page() == LAST_PAGE_ID) {
        return m_pages[LAST_PAGE_ID].is_memory_available();
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

segregated_list::~segregated_list()
{
    segregated_list_element* sle = m_first;
    while (sle) {
        segregated_list_element* next = sle->get_next();
        delete sle;
        sle = next;
    }
}

allocate_result segregated_list::allocate()
{
    if (!m_first) {
        m_first = new segregated_list_element(m_alloc_size);
        m_last = m_first;
    }
    if (!m_first->is_memory_available()) {
        segregated_list_element* new_sle = new segregated_list_element(m_alloc_size, m_first);
        m_first->set_prev(new_sle);
        m_first = new_sle;
    }
    return m_first->allocate();
}

}}