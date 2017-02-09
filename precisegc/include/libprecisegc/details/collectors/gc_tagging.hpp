#ifndef DIPLOMA_GC_TAGGING_HPP
#define DIPLOMA_GC_TAGGING_HPP

#include <cassert>

#include <libprecisegc/gc_common.hpp>
#include <libprecisegc/details/collectors/dptr_descriptor.hpp>

namespace precisegc { namespace details { namespace collectors {

class gc_tagging
{
    static const std::uintptr_t ROOT_BIT = 1;
    static const std::uintptr_t DERIVED_BIT = 2;

    static const std::uintptr_t ALL_BITS = ROOT_BIT | DERIVED_BIT;

    static byte* clear_bits(byte* ptr)
    {
        return reinterpret_cast<byte*>(reinterpret_cast<std::uintptr_t>(ptr) & ~ALL_BITS);
    }

    static byte* get_origin_indirect(byte* ptr)
    {
        return get_dptr_descriptor(ptr)->m_origin;
    }

    static byte* get_derived_indirect(byte* ptr)
    {
        return get_dptr_descriptor(ptr)->m_derived;
    }

    static byte* set_derived_bit(byte* ptr)
    {
        return reinterpret_cast<byte*>(reinterpret_cast<std::uintptr_t>(ptr) | DERIVED_BIT);
    }
public:
    static byte* get(byte* ptr)
    {
        return is_derived(ptr) ? get_derived_indirect(ptr) : clear_bits(ptr);
    }

    static byte* get_obj_start(byte* ptr)
    {
        return is_derived(ptr) ? get_origin_indirect(ptr) : clear_bits(ptr);
    }

    static dptr_descriptor* get_dptr_descriptor(byte* ptr)
    {
        assert(is_derived(ptr));
        return reinterpret_cast<dptr_descriptor*>(clear_bits(ptr));
    }

    static byte* set_root_bit(byte* ptr, bool root_bit)
    {
        return root_bit ? reinterpret_cast<byte*>(reinterpret_cast<std::uintptr_t>(ptr) | ROOT_BIT)
                        : ptr;
    }

    static byte* clear_root_bit(byte* ptr)
    {
        return reinterpret_cast<byte*>(reinterpret_cast<std::uintptr_t>(ptr) & ~ROOT_BIT);
    }

    static byte* make_derived_ptr(dptr_descriptor* descr, bool root_bit)
    {
        return root_bit ? reinterpret_cast<byte*>(reinterpret_cast<std::uintptr_t>(descr) | ALL_BITS)
                        : reinterpret_cast<byte*>(reinterpret_cast<std::uintptr_t>(descr) | DERIVED_BIT);
    }

    static bool is_root(byte* ptr)
    {
        return reinterpret_cast<std::uintptr_t>(ptr) & ROOT_BIT;
    }

    static bool is_derived(byte* ptr)
    {
        return reinterpret_cast<std::uintptr_t>(ptr) & DERIVED_BIT;
    }

    static void reset_derived_ptr(byte* ptr, ptrdiff_t offset)
    {
        assert(is_derived(ptr));
        get_dptr_descriptor(ptr)->m_derived += offset;
    }
};

}}}

#endif //DIPLOMA_GC_TAGGING_HPP
