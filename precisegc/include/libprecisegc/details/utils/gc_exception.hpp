#ifndef DIPLOMA_GC_EXCEPTION_HPP
#define DIPLOMA_GC_EXCEPTION_HPP

#include <exception>
#include <string>

namespace precisegc { namespace details { namespace utils {

class gc_exception : public std::exception
{
public:
    gc_exception(const char* msg)
        : m_msg(std::string("GC exception: ") + msg)
    {}

    const char* what() const noexcept override
    {
        return m_msg.c_str();
    }
private:
    std::string m_msg;
};

}}}

#endif //DIPLOMA_GC_EXCEPTION_HPP
