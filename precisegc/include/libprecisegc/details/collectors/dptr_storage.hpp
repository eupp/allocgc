#ifndef DIPLOMA_DPTR_STORAGE_HPP
#define DIPLOMA_DPTR_STORAGE_HPP

#include <mutex>

#include <libprecisegc/details/gc_word.hpp>
#include <libprecisegc/details/allocators/pool.hpp>

namespace precisegc { namespace details { namespace collectors {

class dptr_storage
{
public:
    static byte* get(byte* ptr);
    static byte* get_origin(byte* ptr);

    static bool is_derived(byte* ptr);

    static void reset_derived_ptr(byte* ptr, ptrdiff_t offset);
    static void forward_derived_ptr(byte* from, byte* to);

    byte* make_derived(byte* ptr, ptrdiff_t offset);

    void destroy_unmarked();
private:
    struct dptr_descriptor
    {
        byte* m_origin;
        byte* m_derived;
    };

    typedef allocators::pool<dptr_descriptor, std::mutex> pool_t;

    static const std::uintptr_t DERIVED_BIT = 1;

    static dptr_descriptor* get_descriptor(byte* ptr);

    static byte* get_origin_indirect(byte* ptr);
    static byte* get_derived_indirect(byte* ptr);

    static byte* set_derived_bit(byte* ptr);
    static byte* clear_derived_bit(byte* ptr);

    pool_t m_pool;
};

}}}

#endif //DIPLOMA_DPTR_STORAGE_HPP
