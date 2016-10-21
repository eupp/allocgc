#include <libprecisegc/details/allocators/mso_allocator.hpp>

#include <libprecisegc/details/collectors/memory_index.hpp>

#include <libprecisegc/details/collectors/traceable_object_meta.hpp>

namespace precisegc { namespace details { namespace allocators {

mso_allocator::mso_allocator()
    : m_freelist(nullptr)
    , m_top(nullptr)
    , m_end(nullptr)
{}

mso_allocator::pointer_type mso_allocator::allocate(size_t size)
{
    if (m_freelist) {
        byte* ptr = m_freelist;
        m_freelist = *reinterpret_cast<byte**>(m_freelist);
        memset(ptr, 0, size);
        return gc_alloc_response(ptr, size, collectors::memory_index::index(ptr));
    }
    if (m_top == m_end) {
        auto new_chunk = create_chunk(size);
        m_top = new_chunk->memory();
        m_end = m_top + new_chunk->size();
    }
    byte* ptr = m_top;
    m_top += size;
    return gc_alloc_response(ptr, size, &m_chunks.back());
}

void mso_allocator::deallocate(pointer_type ptr, size_t size)
{
    return;
}

gc_heap_stat mso_allocator::collect()
{
    gc_heap_stat stats;
    stats.mem_shrunk = 0;
    stats.residency  = 0;

    byte* curr = m_freelist;
    while (curr) {
        managed_pool_chunk* chk = static_cast<managed_pool_chunk*>(collectors::memory_index::index(curr));
        chk->set_live(curr, false);
        curr = *reinterpret_cast<byte**>(curr);
    }

    for (auto it = m_chunks.begin(), end = m_chunks.end(); it != end; ) {
        stats.residency += it->residency();
        if (it->unused()) {
            stats.mem_shrunk += it->size();
            call_destructors(it);
            it = destroy_chunk(it);
        } else {
            ++it;
        }
    }
    stats.residency /= m_chunks.size();
    return stats;
}

void mso_allocator::sweep()
{
    if (m_chunks.empty()) {
        return;
    }
    m_freelist = nullptr;
    auto last = std::prev(m_chunks.end());
    for (auto chunk = m_chunks.begin(); chunk != last; ++chunk) {
        sweep_chunk(chunk, chunk->memory(), chunk->memory() + chunk->size());
    }
    sweep_chunk(last, last->memory(), m_top);
}

void mso_allocator::sweep_chunk(iterator_t chk, byte* mem_start, byte* mem_end)
{
    byte* it = mem_start;
    size_t cnt = (mem_end - it) / chk->cell_size(nullptr);
    size_t cell_size = chk->cell_size(nullptr);

    for (size_t i = 0; i < cnt; ++i, it += cell_size) {
        if (!chk->get_mark(i)) {
            logging::debug() << "Deallocate cell addr=" << (void*) it << ", size=" << cell_size;

            call_destructor(it, chk);

            byte** next = reinterpret_cast<byte**>(it);
            *next = m_freelist;
            m_freelist = it;
        }
    }

    chk->unmark();
}

void mso_allocator::call_destructor(byte* ptr, iterator_t chk)
{
    using namespace collectors;

    if (!chk->is_live(ptr)) {
//        logging::debug() << "Call destructor addr=" << (void*) ptr;

        traceable_object_meta* meta = reinterpret_cast<traceable_object_meta*>(ptr);
        const gc_type_meta* tmeta = meta->get_type_meta();
        byte* obj = ptr + sizeof(traceable_object_meta);

        auto offsets = tmeta->offsets();
        for (size_t offset: offsets) {
            gc_word* word = reinterpret_cast<gc_word*>(obj + offset);
            gc_handle_access::set<std::memory_order_relaxed>(*word, nullptr);
        }

        tmeta->destroy(ptr + sizeof(traceable_object_meta));
    }
}

void mso_allocator::call_destructors(iterator_t chk)
{
    byte* it = chk->memory();
    size_t cnt = chk->size() / chk->cell_size(nullptr);
    size_t cell_size = chk->cell_size(nullptr);

    for (size_t i = 0; i < cnt; ++i, it += cell_size) {
        call_destructor(it, chk);
    }
}

bool mso_allocator::empty() const
{
    return m_chunks.empty();
}

double mso_allocator::residency() const
{
    double r = 0;
    for (auto& chk: m_chunks) {
        r += chk.residency();
    }
    return r / m_chunks.size();
}

mso_allocator::iterator_t mso_allocator::create_chunk(size_t cell_size)
{
    auto alloc_res = allocate_block(cell_size);
    m_chunks.emplace_back(alloc_res.first, alloc_res.second, cell_size);
    return std::prev(m_chunks.end());
}

mso_allocator::iterator_t mso_allocator::destroy_chunk(typename chunk_list_t::iterator chk)
{
    deallocate_block(chk->memory(), chk->size());
    return m_chunks.erase(chk);
}

std::pair<byte*, size_t> mso_allocator::allocate_block(size_t cell_size)
{
    assert(PAGE_SIZE % cell_size == 0);
    size_t chunk_size = chunk_t::chunk_size(cell_size);
    return std::make_pair(core_allocator::allocate(chunk_size), chunk_size);
}

void mso_allocator::deallocate_block(byte* ptr, size_t size)
{
    assert(ptr);
    core_allocator::deallocate(ptr, size);
}

}}}
