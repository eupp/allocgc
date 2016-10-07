#include <libprecisegc/details/allocators/mso_allocator.hpp>

#include <libprecisegc/details/collectors/memory_index.hpp>

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
        return gc_alloc_descriptor(ptr, size, collectors::memory_index::index(ptr));
    }
    if (m_top == m_end) {
        auto new_chunk = create_chunk(size);
        m_top = new_chunk->memory();
        m_end = m_top + new_chunk->size();
    }
    byte* ptr = m_top;
    m_top += size;
    return gc_alloc_descriptor(ptr, size, &m_chunks.back());
}

void mso_allocator::deallocate(pointer_type ptr, size_t size)
{
    return;
}

size_t mso_allocator::shrink()
{
    size_t shrunk = 0;
    for (auto it = m_chunks.begin(), end = m_chunks.end(); it != end; ) {
        if (it->all_unmarked()) {
            shrunk += it->size();
            it = destroy_chunk(it);
        } else {
            ++it;
        }
    }
    return shrunk;
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
    size_t cnt = (mem_end - it) / chk->cell_size();
    size_t cell_size = chk->cell_size();

    for (size_t i = 0; i < cnt; ++i, it += cell_size) {
        if (!chk->get_mark(i)) {
            logging::debug() << "Deallocate cell addr=" << (void*) it << ", size=" << cell_size;

            byte** next = reinterpret_cast<byte**>(it);
            *next = m_freelist;
            m_freelist = it;
        }
    }

    chk->unmark();
}

bool mso_allocator::empty() const
{
    return m_chunks.empty();
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
