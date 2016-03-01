#ifndef DIPLOMA_NONCOPYABLE_H
#define DIPLOMA_NONCOPYABLE_H

#include <type_traits>

namespace precisegc { namespace details {

template <typename... >
using void_t = void;

class noncopyable
{
public:
    noncopyable() = default;

    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;

    noncopyable(noncopyable&&) = default;
    noncopyable& operator=(noncopyable&&) = default;
};

class nonmovable
{
public:
    nonmovable() = default;

    nonmovable(const nonmovable&) = default;
    nonmovable& operator=(const nonmovable&) = default;

    nonmovable(nonmovable&&) = delete;
    nonmovable& operator=(nonmovable&&) = delete;
};

template <typename... Bases>
class ebo : private Bases...
{
public:
    template <typename Base>
    Base& get_base()
    {
        static_assert(std::is_base_of<Base, ebo>::value, "Inappropriate base class");
        return (*this);
    }

    template <typename Base>
    const Base& get_base() const
    {
        static_assert(std::is_base_of<Base, ebo>::value, "Inappropriate base class");
        return (*this);
    }
};

template <bool Cond, typename T>
struct conditional_member {};

template <typename T>
struct conditional_member<true, T>
{
    T member;
};

}}

#endif //DIPLOMA_NONCOPYABLE_H
