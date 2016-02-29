#ifndef DIPLOMA_NONCOPYABLE_H
#define DIPLOMA_NONCOPYABLE_H

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

}}

#endif //DIPLOMA_NONCOPYABLE_H
