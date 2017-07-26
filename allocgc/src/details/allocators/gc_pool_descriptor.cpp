#include <liballocgc/details/allocators/gc_pool_descriptor.hpp>

#include <liballocgc/details/allocators/gc_box.hpp>

namespace allocgc { namespace details { namespace allocators {

gc_pool_descriptor::gc_pool_descriptor(byte* chunk, size_t size, size_t cell_size)
    : m_memory(chunk)
    , m_size(size)
    , m_cell_size_log2(log2(cell_size))
    , m_init_bits(CHUNK_MAXSIZE)
    , m_pin_bits(CHUNK_MAXSIZE)
    , m_mark_bits(CHUNK_MAXSIZE)
{}

gc_pool_descriptor::~gc_pool_descriptor()
{}

double gc_pool_descriptor::residency() const
{
    return static_cast<double>(m_mark_bits.count()) / m_mark_bits.size();
}

//gc_pool_descriptor::memory_range_type gc_pool_descriptor::memory_range()
//{
//    return boost::make_iterator_range(begin(), end());
//}
//
//gc_pool_descriptor::iterator gc_pool_descriptor::begin()
//{
//    return iterator(memory(), this, box_size());
//}
//
//gc_pool_descriptor::iterator gc_pool_descriptor::end()
//{
//    return iterator(memory() + size(), this, box_size());
//}

gc_memory_descriptor::box_id gc_pool_descriptor::get_id(byte* ptr) const
{
    assert(contains(ptr));
    std::uintptr_t uintptr = reinterpret_cast<std::uintptr_t>(ptr);
    uintptr &= ~((1ull << m_cell_size_log2) - 1);
    assert(uintptr % cell_size() == 0);
    return reinterpret_cast<byte*>(uintptr);
}

bool gc_pool_descriptor::is_init(box_id id) const
{
    return m_init_bits.get(calc_box_idx(id));
}

bool gc_pool_descriptor::get_mark(box_id id) const
{
    assert(contains(id));
    assert(is_correct_id(id));
    return m_mark_bits.get(calc_box_idx(id));
}

bool gc_pool_descriptor::get_pin(box_id id) const
{
    assert(contains(id));
    assert(is_correct_id(id));
    return m_pin_bits.get(calc_box_idx(id));
}

void gc_pool_descriptor::set_mark(box_id id, bool mark)
{
    assert(contains(id));
    assert(is_correct_id(id));
    m_mark_bits.set(calc_box_idx(id), mark);
}

void gc_pool_descriptor::set_pin(box_id id, bool pin)
{
    assert(contains(id));
    assert(is_correct_id(id));
    m_pin_bits.set(calc_box_idx(id), pin);
}

gc_lifetime_tag gc_pool_descriptor::get_lifetime_tag(box_id id) const
{
    assert(contains(id));
    assert(is_correct_id(id));
    size_t idx = calc_box_idx(id);
    return get_lifetime_tag_by_bits(get_mark(idx), is_init(idx));
}

size_t gc_pool_descriptor::box_size(box_id id) const
{
    assert(contains(id));
    assert(is_correct_id(id));
    return cell_size();
}

byte* gc_pool_descriptor::box_addr(box_id id) const
{
    assert(contains(id));
    assert(is_correct_id(id));
    return calc_box_addr(id);
}

size_t gc_pool_descriptor::object_count(box_id id) const
{
    assert(contains(id));
    assert(is_correct_id(id));
    return gc_box::get_obj_count(calc_box_addr(id));
}

const gc_type_meta* gc_pool_descriptor::get_type_meta(box_id id) const
{
    assert(contains(id));
    assert(is_correct_id(id));
    return gc_box::get_type_meta(calc_box_addr(id));
}

void gc_pool_descriptor::commit(box_id id)
{
    assert(contains(id));
    assert(is_correct_id(id));
    set_init(calc_box_idx(id), true);
}

void gc_pool_descriptor::commit(box_id id, const gc_type_meta* type_meta)
{
    assert(type_meta);
    assert(is_correct_id(id));
    gc_box::set_type_meta(box_addr(id), type_meta);
    set_init(calc_box_idx(id), true);
}

void gc_pool_descriptor::trace(box_id id, const gc_trace_callback& cb) const
{
    assert(is_correct_id(id));
    gc_box::trace(calc_box_addr(id), cb);
}

//void gc_pool_descriptor::move(byte* to, byte* from, gc_memory_descriptor* from_descr)
//{
//    assert(contains(to));
//    assert(to == box_addr(to));
//    assert(get_lifetime_tag(to) == gc_lifetime_tag::FREE);
//    assert(from_descr->get_lifetime_tag(from) == gc_lifetime_tag::LIVE);
//    gc_box::move(to, from, from_descr->object_count(from), from_descr->get_type_meta(from));
//    from_descr->set_mark(from, false);
//    size_t idx = calc_cell_ind(to);
//    set_mark(idx, true);
//    set_init(idx, true);
//}

void gc_pool_descriptor::finalize(size_t idx)
{
    byte* box_addr = m_memory + idx * cell_size();
    assert(get_lifetime_tag(box_addr) == gc_lifetime_tag::GARBAGE);
    gc_box::destroy(box_addr);
    set_init(idx, false);
}

void gc_pool_descriptor::finalize(box_id id)
{
    assert(is_correct_id(id));
    assert(get_lifetime_tag(id) == gc_lifetime_tag::GARBAGE);
    gc_box::destroy(calc_box_addr(id));
    set_init(calc_box_idx(id), false);
}

//size_t gc_pool_descriptor::calc_cell_ind(byte* ptr) const
//{
//    assert(contains(ptr));
//    assert(ptr == box_addr(ptr));
//    return (ptr - memory()) >> m_cell_size_log2;
//}

size_t gc_pool_descriptor::mem_used()
{
    size_t used = 0;
//    for (auto it = begin(); it != end(); ++it) {
//        if (it->is_init()) {
//            used += it->object_count() * it->get_type_meta()->type_size();
//        }
//    }
    return used;
}

bool gc_pool_descriptor::contains(byte* ptr) const
{
    byte* mem_begin = memory();
    byte* mem_end   = memory() + size();
    return (mem_begin <= ptr) && (ptr < mem_end);
}

bool gc_pool_descriptor::is_correct_id(box_id id) const
{
    return id == get_id(id);
}

}}}
