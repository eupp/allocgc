#ifndef DIPLOMA_NONCOPYABLE_H
#define DIPLOMA_NONCOPYABLE_H

namespace precisegc { namespace details {

class noncopyable
{
public:
    noncopyable() = default;

    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;

    noncopyable(noncopyable&&) = default;
    noncopyable& operator=(noncopyable&&) = default;
};

}}

#endif //DIPLOMA_NONCOPYABLE_H
