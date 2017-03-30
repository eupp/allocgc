#ifndef ALLOCGC_SYS_ALLOCATOR_HPP
#define ALLOCGC_SYS_ALLOCATOR_HPP

#include <cstddef>
#include <cstdint>
#include <cstdlib>

#include <sys/mman.h>

#include <boost/range/iterator_range.hpp>

#include <liballocgc/details/allocators/allocator_tag.hpp>
#include <liballocgc/details/utils/block_ptr.hpp>
#include <liballocgc/details/logging.hpp>
#include <liballocgc/details/constants.hpp>
#include <liballocgc/gc_common.hpp>

namespace allocgc { namespace details { namespace allocators {

class sys_allocator
{
public:
    typedef byte* pointer_type;
    typedef stateless_alloc_tag alloc_tag;
    typedef boost::iterator_range<byte*> memory_range_type;

    static size_t align_size(size_t size)
    {
        return ((size + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;
    }

    static byte* allocate(size_t size)
    {
        assert(size % PAGE_SIZE == 0);
        void* mem = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (mem == MAP_FAILED) {
            logging::warning() << "mmap failed: " << strerror(errno);
            assert(false);
        }
        return reinterpret_cast<byte*>(mem);
    }

    static void deallocate(byte* ptr, size_t size)
    {
        int ret = munmap(reinterpret_cast<void*>(ptr), size);
        if (ret == -1) {
            logging::error() << "munmap failed: " << strerror(errno);
        }
    }

    static size_t shrink()
    {
        return 0;
    }

    static memory_range_type memory_range()
    {
        return memory_range_type(nullptr, nullptr);
    }
};

}}}

#endif //ALLOCGC_SYS_ALLOCATOR_HPP
