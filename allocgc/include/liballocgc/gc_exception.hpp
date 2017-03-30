#ifndef ALLOCGC_GC_EXCEPTION_HPP
#define ALLOCGC_GC_EXCEPTION_HPP

#include <exception>
#include <string>

namespace allocgc {

class gc_exception : public std::exception
{
public:
    gc_exception(const char* msg)
        : m_msg(std::string("GC exception: ") + msg)
    {}

    gc_exception(const std::string& msg)
        : m_msg("GC exception: " + msg)
    {}

    const char* what() const noexcept override
    {
        return m_msg.c_str();
    }
private:
    std::string m_msg;
};

class gc_bad_alloc : public gc_exception
{
public:
    gc_bad_alloc()
        : gc_exception("managed heap failed to allocate memory;")
    {}
};

}

#endif //ALLOCGC_GC_EXCEPTION_HPP
