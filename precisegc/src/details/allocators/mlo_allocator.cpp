#include <libprecisegc/details/allocators/mlo_allocator.hpp>

namespace precisegc { namespace details { namespace allocators {

gc_alloc_response mlo_allocator::allocate(const gc_alloc_request& rqst)
{
    size_t chunk_size = descriptor_t::chunk_size(rqst.alloc_size());
    size_t memblk_size = chunk_size + sizeof(control_block) + sizeof(descriptor_t);

    auto deleter = [this, memblk_size] (byte* ptr) {
        deallocate_block(ptr, memblk_size);
    };
    std::unique_ptr<byte, decltype(deleter)> memblk_owner(allocate_block(memblk_size), deleter);
    byte*  memblk = memblk_owner.get();

    control_block* fake  = get_fake_block();
    control_block* cblk  = get_control_block(memblk);
    descriptor_t*  descr = get_descriptor(memblk);

    new (descr) descriptor_t(get_mem(memblk), chunk_size, cell_size);

    cblk->m_next = fake;
    cblk->m_prev = fake->m_prev;

    fake->m_prev->m_next = cblk;
    fake->m_prev = cblk;

    if (m_head == fake) {
        m_head = cblk;
    }
    memblk_owner.release();

    return gc_alloc_response(get_mem(memblk), rqst.alloc_size(), descr);
}

gc_heap_stat mlo_allocator::collect(compacting::forwarding& frwd)
{
    gc_heap_stat stat;

    for (auto it = begin(); it != end(); ) {
        auto next = std::next(it);
        stat.mem_used += it->size();
        if (!it->get_mark()) {
            stat.mem_freed += it->size();
            destroy(it->memory());
        }
        it = next;
    }
    stat.mem_residency = 1;

    return stat;
}

control_block* mlo_allocator::get_fake_block()
{
    return &m_fake;
}

mlo_allocator::control_block * mlo_allocator::get_control_block(byte* memblk)
{
    assert(memblk);
    return reinterpret_cast<control_block*>(memblk);
}

mlo_allocator::descriptor_t* mlo_allocator::get_descriptor(byte* memblk)
{
    assert(memblk);
    return reinterpret_cast<descriptor_t*>(memblk + sizeof(control_block));
}

byte * mlo_allocator::get_mem(byte* memblk)
{
    assert(memblk);
    return memblk + sizeof(control_block) + sizeof(descriptor_t);
}

}}}
