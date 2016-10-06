#include <libprecisegc/details/collectors/indexed_managed_object.hpp>

#include <cassert>
#include <utility>

#include <libprecisegc/details/collectors/memory_index.hpp>
#include <libprecisegc/details/collectors/dptr_storage.hpp>
#include <libprecisegc/details/collectors/traceable_object_meta.hpp>

namespace precisegc { namespace details { namespace collectors {

indexed_managed_object indexed_managed_object::index(byte* ptr)
{
    assert(!collectors::dptr_storage::is_derived(ptr));

    memory_descriptor* descr = memory_index::index(ptr);
    if (ptr && !descr) {
        throw unindexed_memory_exception(ptr);
    }
    return indexed_managed_object(ptr, descr);
}

indexed_managed_object indexed_managed_object::index_by_indirect_ptr(byte* ptr)
{
    assert(!collectors::dptr_storage::is_derived(ptr));

    if (!ptr) {
        return indexed_managed_object(nullptr, nullptr);
    }

    memory_descriptor* descr = memory_index::index(ptr);
    if (ptr && !descr) {
        throw unindexed_memory_exception(ptr);
    }
    byte* cell_start = managed_object::get_object(descr->cell_start(ptr));
    return indexed_managed_object(cell_start, descr);
}

bool indexed_managed_object::get_mark(byte* ptr)
{
    return indexed_managed_object::index(ptr).get_mark();
}

void indexed_managed_object::set_mark(byte* ptr, bool mark)
{
    indexed_managed_object::index(ptr).set_mark(mark);
}

bool indexed_managed_object::get_pin(byte* ptr)
{
    return indexed_managed_object::index(ptr).get_pin();
}

void indexed_managed_object::set_pin(byte* ptr, bool pin)
{
    indexed_managed_object::index(ptr).set_pin(pin);
}

traceable_object_meta* indexed_managed_object::get_meta(byte* ptr)
{
    return indexed_managed_object::index(ptr).meta();
}

indexed_managed_object::indexed_managed_object(byte* ptr, memory_descriptor* descriptor)
    : m_obj(ptr)
    , m_descr(descriptor)
{
    assert(!collectors::dptr_storage::is_derived(ptr));
    assert(memory_index::index(ptr) == m_descr || memory_index::index(ptr - PAGE_SIZE) == m_descr);
}

bool indexed_managed_object::get_mark() const
{
    assert(m_descr);
    return m_descr->get_mark(m_obj.object());
}

bool indexed_managed_object::get_pin() const
{
    assert(m_descr);
    return m_descr->get_pin(m_obj.object());
}

void indexed_managed_object::set_mark(bool mark) const
{
    assert(m_descr);
    return m_descr->set_mark(m_obj.object(), mark);
}

void indexed_managed_object::set_pin(bool pin) const
{
    assert(m_descr);
    return m_descr->set_pin(m_obj.object(), pin);
}

size_t indexed_managed_object::cell_size() const
{
    assert(m_descr);
    return m_descr->cell_size();
}

traceable_object_meta* indexed_managed_object::meta() const
{
    assert(m_descr);
    return m_obj.meta();
}

byte* indexed_managed_object::object() const
{
    assert(m_descr);
    return m_obj.object();
}

memory_descriptor* indexed_managed_object::descriptor() const
{
    return m_descr;
}

indexed_managed_object::operator bool() const
{
    return static_cast<bool>(m_obj);
}

}}}