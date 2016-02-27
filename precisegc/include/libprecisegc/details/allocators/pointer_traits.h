#ifndef DIPLOMA_POINTER_TRAITS_H
#define DIPLOMA_POINTER_TRAITS_H

#include <memory>
#include <type_traits>

namespace precisegc { namespace details { namespace allocators {

template <typename Ptr>
class pointer_traits
{
    typedef typename std::pointer_traits<Ptr>::element_type element_type;
public:
    template <typename T = Ptr>
    static auto member_of_operator(Ptr& ptr)
        -> typename std::enable_if<std::is_same<T, element_type*>::value, element_type*>::type
    {
        return ptr;
    }

    template <typename T = Ptr>
    static auto member_of_operator(Ptr& ptr)
        -> typename std::enable_if<!std::is_same<T, element_type*>::value, element_type*>::type
    {
        return ptr.operator->();
    }

    template <typename T = Ptr>
    static auto get(Ptr& ptr)
        -> std::enable_if<std::is_same<T, element_type*>::value, element_type*>
    {
        return ptr;
    }

    template <typename T = Ptr>
    static auto get(Ptr& ptr)
        -> std::enable_if<!std::is_same<T, element_type*>::value, element_type*>
    {
        return ptr.get();
    }

    template <typename T = Ptr>
    static auto reset(Ptr& ptr)
        -> std::enable_if<std::is_same<T, element_type*>::value, void>
    {
        ptr = nullptr;
    }

    template <typename T = Ptr>
    static auto reset(Ptr& ptr)
        -> std::enable_if<!std::is_same<T, element_type*>::value, void>
    {
        ptr.reset();
    }

    template <typename T = Ptr>
    static auto reset(Ptr& ptr, Ptr new_ptr)
        -> std::enable_if<std::is_same<T, element_type*>::value, void>
    {
        ptr = new_ptr;
    }

    template <typename T = Ptr>
    static auto reset(Ptr& ptr, Ptr new_ptr)
        -> std::enable_if<!std::is_same<T, element_type*>::value, void>
    {
        ptr.reset(new_ptr);
    }
};

}}}

#endif //DIPLOMA_POINTER_TRAITS_H
