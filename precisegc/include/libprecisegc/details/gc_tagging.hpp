#ifndef DIPLOMA_GC_TAGGING_HPP
#define DIPLOMA_GC_TAGGING_HPP

#include <libprecisegc/details/types.hpp>

namespace precisegc { namespace details {

class gc_tagging
{
public:
    static bool derived_bit(byte* ptr)
    {
        return reinterpret_cast<byte*>(reinterpret_cast<std::uintptr_t>(ptr) & DERIVED_BIT);
    }

    static void set_derived_bit(byte*& ptr)
    {
        ptr = reinterpret_cast<byte*>(reinterpret_cast<std::uintptr_t>(ptr) | DERIVED_BIT);
    }

    static byte* clear(byte* ptr)
    {
        return reinterpret_cast<byte*>(reinterpret_cast<std::uintptr_t>(ptr) & ~DERIVED_BIT);
    }
private:
    static const std::uintptr_t DERIVED_BIT = 1;
};

}}

#endif //DIPLOMA_GC_TAGGING_HPP
