#ifndef DIPLOMA_MAKE_UNIQUE_HPP
#define DIPLOMA_MAKE_UNIQUE_HPP

#include <memory>

namespace precisegc { namespace details { namespace utils {

namespace internals {
template<typename T>
struct make_unique_if
{
    typedef std::unique_ptr<T> single_object;
};

template<typename T>
struct make_unique_if<T[]>
{
    typedef std::unique_ptr<T[]> unknown_bound;
};

template<typename T, size_t N>
struct make_unique_if<T[N]>
{
    typedef void known_bound;
};
}

template <typename T, typename... Args>
auto make_unique(Args&&... args)
    -> typename internals::make_unique_if<T>::single_object
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
};

template <typename T>
auto make_unique(size_t n)
    -> typename internals::make_unique_if<T>::unknown_bound
{
    typedef typename std::remove_extent<T>::type U;
    return std::unique_ptr<T>(new U[n]);
};

template<typename T, typename... Args>
auto make_unique(Args&&...)
    -> typename internals::make_unique_if<T>::known_bound
    = delete;

}}}

#endif //DIPLOMA_MAKE_UNIQUE_HPP
