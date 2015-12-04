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

void segregated_list_element::clear(const page_descriptor::iterator &it, size_t page_id)
{
    m_pages[page_id].clear(it);
    set_last_used_page(page_id);
    for (size_t i = page_id + 1; i < PAGES_PER_SEGREGATED_STORAGE_ELEMENT; ++i) {
        page_descriptor& pd = m_pages[i];
        pd.clear(pd.begin());
    }
}

void segregated_list_element::clear_mark_bits() noexcept
{
    for (size_t i = 0; i < LAST_PAGE_ID; ++i) {
        m_pages[i].clear_mark_bits();
    }
}

void segregated_list_element::clear_pin_bits() noexcept
{
    for (size_t i = 0; i < LAST_PAGE_ID; ++i) {
        m_pages[i].clear_pin_bits();
    }
}

page_descriptor& segregated_list_element::get_page_descriptor(size_t ind)
{
    assert(ind < PAGES_PER_SEGREGATED_STORAGE_ELEMENT);
    return m_pages[ind];
}

size_t segregated_list_element::last_used_page() const noexcept
{
    return m_header.m_last_used_page;
}

void segregated_list_element::set_last_used_page(size_t id) noexcept
{
    m_header.m_last_used_page = id;
}

bool segregated_list_element::is_memory_available() const noexcept
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
    clear(begin());
}

allocate_result segregated_list::allocate()
{
    if (!m_last) {
        m_first = new segregated_list_element(m_alloc_size);
        m_last = m_first;
    }
    if (!m_last->is_memory_available()) {
        segregated_list_element* new_sle = new segregated_list_element(m_alloc_size, nullptr, m_last);
        m_last->set_next(new_sle);
        m_last = new_sle;
    }
    return m_last->allocate();
}

segregated_list::iterator segregated_list::begin() noexcept
{
    if (!m_first) {
        return iterator(nullptr, 0, page_descriptor::iterator());
    }
    return iterator(m_first, 0, m_first->get_page_descriptor(0).begin());
}

segregated_list::iterator segregated_list::end() noexcept
{
    if (!m_first) {
        return iterator(nullptr, 0, page_descriptor::iterator());
    }
    size_t page_id = m_last->last_used_page();
    return iterator(m_last, page_id, m_last->get_page_descriptor(page_id).end());
}

void segregated_list::compact(forwarding_list& forwarding)
{
    if (!m_first) {
        return;
    }
    iterator from = end();
    --from;
    iterator to = begin();
    iterator last_pined = begin();
    while (from != to) {
        while (!from.is_marked() && from != to) {
            --from;
        }
        while (to.is_marked() && from != to) {
            ++to;
        }
        if (from.is_pinned()) {
            if (last_pined == begin()) {
                last_pined = from;
            }
            --from;
            continue;
        }
        if (from != to) {
            forwarding.emplace_back(*from, *to);
        }
        --from;
        if (from == to) {
            break;
        } else {
            to++;
        }
    }
    iterator clear_it = last_pined;
    if (clear_it.is_marked()) {
        ++clear_it;
    }
    clear(clear_it);
    clear_mark_bits();
}

void segregated_list::clear(const iterator &it)
{
    if (!m_first) {
        return;
    }

    // delete all elements after current
    segregated_list_element* sle = it.m_sle->get_next();
    while (sle) {
        segregated_list_element* next = sle->get_next();
        delete sle;
        sle = next;
    }

    if (it != begin()) {
        it.m_sle->set_next(nullptr);
        it.m_sle->clear(it.m_pd_itr, it.m_pd_ind);
        m_last = it.m_sle;
    } else {
        delete m_first;
        m_first = m_last = nullptr;
    }
}

void segregated_list::clear_mark_bits() noexcept
{
    if (!m_first) {
        return;
    }
    segregated_list_element* sle = m_first;
    while (sle) {
        sle->clear_mark_bits();
        sle = sle->get_next();
    }
}

void segregated_list::clear_pin_bits() noexcept
{
    if (!m_first) {
        return;
    }
    segregated_list_element* sle = m_first;
    while (sle) {
        sle->clear_pin_bits();
        sle = sle->get_next();
    }
}

segregated_list::iterator::iterator(segregated_list_element* sle,
                                    size_t pd_ind,
                                    page_descriptor::iterator pd_itr) noexcept
    : m_sle(sle)
    , m_pd_ind(pd_ind)
    , m_pd_itr(pd_itr)
{}

void* const segregated_list::iterator::operator*() const noexcept
{
    return *m_pd_itr;
}

segregated_list::iterator segregated_list::iterator::operator++() noexcept
{
    assert(m_sle);
    ++m_pd_itr;
    if (m_pd_itr == m_sle->get_page_descriptor(m_pd_ind).end()) {
        // if it's last page and there is no next then we reach the end of all memory
        if (m_pd_ind == m_sle->last_used_page() && !m_sle->get_next()) {
            return *this;
        }
        if (m_pd_ind == LAST_PAGE_ID) {
            assert(m_sle->get_next());
            m_sle = m_sle->get_next();
            m_pd_ind = 0;
        } else {
            ++m_pd_ind;
        }
        m_pd_itr = m_sle->get_page_descriptor(m_pd_ind).begin();
    }
    return *this;
}

segregated_list::iterator segregated_list::iterator::operator++(int) noexcept
{
    iterator it = *this;
    ++(*this);
    return it;
}

segregated_list::iterator segregated_list::iterator::operator--() noexcept
{
    if (m_pd_itr == m_sle->get_page_descriptor(m_pd_ind).begin()) {
        if (m_pd_ind == 0) {
            assert(m_sle->get_prev());
            m_sle = m_sle->get_prev();
            m_pd_ind = LAST_PAGE_ID;
        } else {
            --m_pd_ind;
        }
        m_pd_itr = m_sle->get_page_descriptor(m_pd_ind).end();
    }
    --m_pd_itr;
    return *this;
}

segregated_list::iterator segregated_list::iterator::operator--(int) noexcept
{
    iterator it = *this;
    --(*this);
    return it;
}

bool operator==(const segregated_list::iterator& it1, const segregated_list::iterator& it2)
{
    return it1.m_pd_itr == it2.m_pd_itr;
}

bool operator!=(const segregated_list::iterator& it1, const segregated_list::iterator& it2)
{
    return !(it1 == it2);
}

bool segregated_list::iterator::is_marked() const noexcept
{
    return m_pd_itr.is_marked();
}


bool segregated_list::iterator::is_pinned() const noexcept
{
    return m_pd_itr.is_pinned();
}

void segregated_list::iterator::set_marked(bool marked) noexcept
{
    m_pd_itr.set_marked(marked);
}

void segregated_list::iterator::set_pinned(bool pinned) noexcept
{
    m_pd_itr.set_pinned(pinned);
}

}}