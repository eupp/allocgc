#ifndef DIPLOMA_GET_PTR_HPP
#define DIPLOMA_GET_PTR_HPP

#include <memory>

namespace precisegc { namespace details { namespace utils {

template <typename Ptr>
using is_raw_ptr = typename std::is_same<Ptr, typename std::pointer_traits<Ptr>::element_type*>::type;

template <typename Ptr, typename = typename std::enable_if<is_raw_ptr<Ptr>::value>::type>
Ptr get_ptr(Ptr p)
{
    return p;
}

template <typename Ptr, typename = typename std::enable_if<!is_raw_ptr<Ptr>::value>::type>
auto get_ptr(const Ptr& p)
    -> typename std::result_of<decltype(&Ptr::get)(Ptr)>::type
{
    return p.get();
}

}}}

#endif //DIPLOMA_GET_PTR_HPP
