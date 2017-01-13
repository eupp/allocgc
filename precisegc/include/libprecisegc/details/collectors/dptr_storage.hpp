#ifndef DIPLOMA_DPTR_STORAGE_HPP
#define DIPLOMA_DPTR_STORAGE_HPP

#include <mutex>
#include <atomic>

#include <libprecisegc/details/collectors/dptr_descriptor.hpp>
#include <libprecisegc/details/allocators/pool.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details { namespace collectors {

class dptr_storage : utils::noncopyable, utils::nonmovable
{
public:
    dptr_storage();

    dptr_descriptor* make_derived(byte* ptr, ptrdiff_t offset);
    void destroy_unmarked();
private:
    typedef allocators::pool<dptr_descriptor, std::mutex> pool_t;

    pool_t m_pool;
    std::atomic<dptr_descriptor*> m_head;
};

}}}

#endif //DIPLOMA_DPTR_STORAGE_HPP
