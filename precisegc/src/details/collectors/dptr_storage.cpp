#include <libprecisegc/details/collectors/dptr_storage.hpp>

#include <libprecisegc/details/allocators/memory_index.hpp>
#include <libprecisegc/details/gc_hooks.hpp>
#include <libprecisegc/details/logging.hpp>

namespace precisegc { namespace details { namespace collectors {

dptr_storage::dptr_storage()
    : m_head(nullptr)
{ }

dptr_descriptor* dptr_storage::make_derived(byte* ptr, ptrdiff_t offset)
{
    dptr_descriptor* head = m_head.load(std::memory_order_acquire);
    dptr_descriptor* dscr = m_pool.create();
    dscr->m_origin  = ptr;
    dscr->m_derived = ptr + offset;
    dscr->m_next    = head;
    m_head.compare_exchange_strong(head, dscr, std::memory_order_acq_rel);
    return dscr;
}

void dptr_storage::destroy_unmarked()
{
    dptr_descriptor* head = m_head.load(std::memory_order_relaxed);

    while (head && !allocators::memory_index::get_gc_cell(head->m_origin).get_mark()) {
        dptr_descriptor* next = head->m_next;
        m_pool.destroy(head);
        head = next;
    }

    m_head.store(head, std::memory_order_relaxed);

    if (!head) {
        return;
    }

    dptr_descriptor* prev = head;
    dptr_descriptor* curr = head->m_next;
    while (curr) {
        if (!allocators::memory_index::get_gc_cell(curr->m_origin).get_mark()) {
            dptr_descriptor* next = curr->m_next;
            m_pool.destroy(curr);
            prev->m_next = next;
            curr = next;
        } else {
            prev = curr;
            curr = curr->m_next;
        }
    }
}

}}}