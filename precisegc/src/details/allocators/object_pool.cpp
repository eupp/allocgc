#include <libprecisegc/details/allocators/object_pool.hpp>

namespace precisegc { namespace details { namespace allocators {

thread_local object_pool::object_pool_t object_pool::pool{};

}}}