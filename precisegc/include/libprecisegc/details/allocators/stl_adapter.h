#ifndef DIPLOMA_STL_ADAPTER_H
#define DIPLOMA_STL_ADAPTER_H

#include <type_traits>
#include <memory>
#include <limits>

#include "types.h"
#include "../util.h"

namespace precisegc { namespace details { namespace allocators {

template<typename T, typename Alloc>
class stl_adapter : private Alloc
{
    static_assert(std::is_same<typename Alloc::pointer_type, byte*>::value, "stl_adapter should be used with raw memory allocator");
public:
    typedef T value_type;

    // stateful allocators should be propagated
    typedef typename std::is_empty<Alloc>::type propagate_on_container_copy_assignment;
    typedef typename std::is_empty<Alloc>::type propagate_on_container_move_assignment;
    typedef typename std::is_empty<Alloc>::type propagate_on_container_swap;

    stl_adapter() = default;
    stl_adapter(const stl_adapter&) = default;
    stl_adapter(stl_adapter&&) = default;

    template <typename U>
    stl_adapter(const stl_adapter<U, Alloc>& other)
        : Alloc(other)
    {}

    template <typename U>
    stl_adapter(const stl_adapter<U, Alloc>&& other)
        : Alloc(std::move(other))
    {}

    stl_adapter& operator=(const stl_adapter&) = default;
    stl_adapter& operator=(stl_adapter&&) = default;

    T* allocate(size_t n)
    {
        return reinterpret_cast<T*>(Alloc::allocate(n * sizeof(T)));
    }

    void deallocate(T* ptr, size_t n)
    {
        Alloc::deallocate(reinterpret_cast<byte*>(ptr), n * sizeof(T));
    }

    static size_t max_size()
    {
        return std::numeric_limits<size_t>::max() / sizeof(T);
    }
};

}}}

#endif //DIPLOMA_STL_ADAPTER_H
