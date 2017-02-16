#include <libprecisegc/details/allocators/gc_pool_descriptor.hpp>

#include <libprecisegc/details/allocators/gc_box.hpp>

namespace precisegc { namespace details { namespace allocators {

gc_pool_descriptor::gc_pool_descriptor(byte* chunk, size_t size, size_t cell_size)
    : m_memory(chunk)
    , m_size(size)
    , m_cell_size_log2(log2(cell_size))
{}

gc_pool_descriptor::~gc_pool_descriptor()
{}

gc_memory_descriptor* gc_pool_descriptor::descriptor()
{
    return this;
}

byte* gc_pool_descriptor::init_cell(byte* ptr, size_t obj_count, const gc_type_meta* type_meta)
{
    assert(contains(ptr));
    assert(ptr == cell_start(ptr));
    return gc_box::create(ptr, obj_count, type_meta);
}

bool gc_pool_descriptor::contains(byte* ptr) const
{
    byte* mem_begin = memory();
    byte* mem_end   = memory() + size();
    return (mem_begin <= ptr) && (ptr < mem_end);
}

bool gc_pool_descriptor::unused() const
{
    return m_mark_bits.none();
}

size_t gc_pool_descriptor::count_lived() const
{
    return m_mark_bits.count();
}

size_t gc_pool_descriptor::count_pinned() const
{
    return m_pin_bits.count();
}

void gc_pool_descriptor::unmark()
{
    m_mark_bits.reset_all();
    m_pin_bits.reset_all();
}

double gc_pool_descriptor::residency() const
{
    return static_cast<double>(m_mark_bits.count()) / m_mark_bits.size();
}

gc_pool_descriptor::memory_range_type gc_pool_descriptor::memory_range()
{
    return boost::make_iterator_range(begin(), end());
}

gc_pool_descriptor::iterator gc_pool_descriptor::begin()
{
    return iterator(memory(), this, cell_size());
}

gc_pool_descriptor::iterator gc_pool_descriptor::end()
{
    return iterator(memory() + size(), this, cell_size());
}

bool gc_pool_descriptor::is_init(byte* ptr) const
{
    assert(contains(ptr));
    assert(ptr == cell_start(ptr));
    size_t idx = calc_cell_ind(ptr);
    return is_init(idx);
}

void gc_pool_descriptor::set_init(byte* ptr, bool init)
{
    assert(contains(ptr));
    assert(ptr == cell_start(ptr));
    size_t idx = calc_cell_ind(ptr);
    set_init(idx, init);
}

bool gc_pool_descriptor::get_mark(byte* ptr) const
{
    assert(contains(ptr));
    assert(ptr == cell_start(ptr));
    size_t idx = calc_cell_ind(ptr);
    return get_mark(idx);
}

bool gc_pool_descriptor::get_pin(byte* ptr) const
{
    assert(contains(ptr));
    assert(ptr == cell_start(ptr));
    size_t idx = calc_cell_ind(ptr);
    return get_pin(idx);
}

void gc_pool_descriptor::set_mark(byte* ptr, bool mark)
{
    assert(contains(ptr));
    assert(ptr == cell_start(ptr));
    size_t idx = calc_cell_ind(ptr);
    set_mark(idx, mark);
}

void gc_pool_descriptor::set_pin(byte* ptr, bool pin)
{
    assert(contains(ptr));
    assert(ptr == cell_start(ptr));
    size_t idx = calc_cell_ind(cell_start(ptr));
    set_pin(idx, pin);
}

gc_lifetime_tag gc_pool_descriptor::get_lifetime_tag(size_t idx) const
{
    return get_lifetime_tag_by_bits(get_mark(idx), is_init(idx));
}

gc_lifetime_tag gc_pool_descriptor::get_lifetime_tag(byte* ptr) const
{
    assert(contains(ptr));
    assert(ptr == cell_start(ptr));
    size_t idx = calc_cell_ind(ptr);
    return get_lifetime_tag(idx);
}

size_t gc_pool_descriptor::cell_size() const
{
    return 1ull << m_cell_size_log2;
}

size_t gc_pool_descriptor::cell_size(byte* ptr) const
{
    assert(contains(ptr));
    return cell_size();
}

byte* gc_pool_descriptor::cell_start(byte* ptr) const
{
    assert(contains(ptr));
    std::uintptr_t uintptr = reinterpret_cast<std::uintptr_t>(ptr);
    uintptr &= ~((1ull << m_cell_size_log2) - 1);
    assert(uintptr % cell_size(ptr) == 0);
    return reinterpret_cast<byte*>(uintptr);
}

size_t gc_pool_descriptor::object_count(byte* ptr) const
{
    assert(contains(ptr));
    assert(ptr == cell_start(ptr));
    return gc_box::get_obj_count(cell_start(ptr));
}

const gc_type_meta* gc_pool_descriptor::get_type_meta(byte* ptr) const
{
    assert(contains(ptr));
    assert(ptr == cell_start(ptr));
    return gc_box::get_type_meta(cell_start(ptr));
}

void gc_pool_descriptor::commit(byte* ptr)
{
    assert(contains(ptr));
    assert(ptr == cell_start(ptr));
    assert(gc_box::get_type_meta(ptr));
    set_init(ptr, true);
}

void gc_pool_descriptor::commit(byte* ptr, const gc_type_meta* type_meta)
{
    assert(type_meta);
    assert(contains(ptr));
    assert(ptr == cell_start(ptr));
    gc_box::set_type_meta(ptr, type_meta);
    set_init(ptr, true);
}

void gc_pool_descriptor::trace(byte* ptr, const gc_trace_callback& cb) const
{
    assert(contains(ptr));
    assert(ptr == cell_start(ptr));
    gc_box::trace(ptr, cb);
}

void gc_pool_descriptor::move(byte* to, byte* from, gc_memory_descriptor* from_descr)
{
    assert(contains(to));
    assert(to == cell_start(to));
    assert(get_lifetime_tag(to) == gc_lifetime_tag::FREE);
    assert(from_descr->get_lifetime_tag(from) == gc_lifetime_tag::LIVE);
    gc_box::move(to, from, from_descr->object_count(from), from_descr->get_type_meta(from));
    from_descr->set_mark(from, false);
    size_t idx = calc_cell_ind(to);
    set_mark(idx, true);
    set_init(idx, true);
}

void gc_pool_descriptor::finalize(size_t i)
{
    assert(get_lifetime_tag(i) == gc_lifetime_tag::GARBAGE);
    gc_box::destroy(m_memory + i * cell_size());
    set_init(i, false);
}

void gc_pool_descriptor::finalize(byte* ptr)
{
    assert(contains(ptr));
    assert(ptr == cell_start(ptr));
    assert(get_lifetime_tag(ptr) == gc_lifetime_tag::GARBAGE);
    gc_box::destroy(ptr);
    set_init(ptr, false);
}

size_t gc_pool_descriptor::calc_cell_ind(byte* ptr) const
{
    assert(contains(ptr));
    assert(ptr == cell_start(ptr));
    return (ptr - memory()) >> m_cell_size_log2;
}

}}}
