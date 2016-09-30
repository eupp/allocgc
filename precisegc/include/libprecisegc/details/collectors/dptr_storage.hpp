#ifndef DIPLOMA_DPTR_STORAGE_HPP
#define DIPLOMA_DPTR_STORAGE_HPP

#include <mutex>

#include <libprecisegc/details/gc_handle.hpp>
#include <libprecisegc/details/allocators/pool.hpp>

namespace precisegc { namespace details { namespace collectors {

class dptr_storage
{
public:
    byte* get(const gc_handle* handle);
    byte* get_origin(const gc_handle* handle);

    byte* make_derived(const gc_handle* handle, ptrdiff_t offset);

    void destroy_unmarked();
private:
    struct dptr_descriptor
    {
        byte* m_origin;
        byte* m_derived;
    };

    typedef allocators::pool<dptr_descriptor, std::mutex> pool_t;

    static const std::uintptr_t DERIVED_BIT = 1;

    static byte* get_origin(byte* ptr);
    static byte* get_derived(byte* ptr);

    static bool  derived_bit(byte* ptr);
    static void  set_derived_bit(byte*& ptr);
    static byte* clear_derived_bit(byte* ptr);

    pool_t m_pool;
};

}}}

#endif //DIPLOMA_DPTR_STORAGE_HPP
