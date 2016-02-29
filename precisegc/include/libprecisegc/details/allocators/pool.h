#ifndef DIPLOMA_POOL_H
#define DIPLOMA_POOL_H

#include <cstddef>

#include "types.h"

namespace precisegc { namespace details { namespace allocators {

template <typename Alloc>
class pool
{
public:

    byte* allocate(size_t size);
    void deallocate(byte* ptr);

private:

};

}}}

#endif //DIPLOMA_POOL_H
