#ifndef DIPLOMA_INDEXED_MANAGED_OBJECT_HPP
#define DIPLOMA_INDEXED_MANAGED_OBJECT_HPP

#include <libprecisegc/details/types.hpp>
#include <libprecisegc/details/gc_exception.hpp>
#include <libprecisegc/details/utils/to_string.hpp>
#include <libprecisegc/details/memory_descriptor.hpp>
#include <libprecisegc/details/collectors/managed_object.hpp>

namespace precisegc { namespace details { namespace collectors {

class unindexed_memory_exception : public gc_exception
{
public:
    unindexed_memory_exception(byte* ptr)
        : gc_exception(("Inappropriate access to unindexed memory by address " + utils::to_string(ptr)).c_str())
    {}
};

class traceable_object_meta;

class indexed_managed_object
{
public:
    typedef byte element_type;

    static indexed_managed_object index(byte* ptr);
    static indexed_managed_object index_by_indirect_ptr(byte* ptr);

    static bool get_mark(byte* ptr);
    static void set_mark(byte* ptr, bool mark);

    static bool get_pin(byte* ptr);
    static void set_pin(byte* ptr, bool pin);

    static traceable_object_meta* get_meta(byte* ptr);

    indexed_managed_object() = delete;

    indexed_managed_object(const indexed_managed_object&) = default;
    indexed_managed_object& operator=(const indexed_managed_object&) = default;

    indexed_managed_object(byte* ptr, memory_descriptor* descriptor);

    bool get_mark() const;
    bool get_pin() const;

    void set_mark(bool mark) const;
    void set_pin(bool pin) const;

    size_t cell_size() const;

    byte* object() const;
    traceable_object_meta* meta() const;
    memory_descriptor* descriptor() const;

    explicit operator bool() const;
private:
    managed_object m_obj;
    memory_descriptor* m_descr;
};

}}}

#endif //DIPLOMA_INDEXED_MANAGED_OBJECT_HPP
