#ifndef DIPLOMA_MSO_ALLOCATOR_HPP
#define DIPLOMA_MSO_ALLOCATOR_HPP

#include <set>

#include <libprecisegc/details/allocators/managed_pool_chunk.hpp>
#include <libprecisegc/details/utils/locked_range.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details { namespace allocators {

class mso_allocator : private utils::noncopyable, private utils::nonmovable
{
public:
    typedef std::set<managed_pool_chunk, stl_adapter<Chunk, >> list_t;
    typedef typename list_t::iterator iterator_t;
private:
};

}}}

#endif //DIPLOMA_MSO_ALLOCATOR_HPP
