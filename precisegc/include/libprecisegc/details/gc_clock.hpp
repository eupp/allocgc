#ifndef DIPLOMA_GC_CLOCK_HPP
#define DIPLOMA_GC_CLOCK_HPP

#include <cstdint>
#include <chrono>

namespace precisegc { namespace details {

class gc_clock
{
public:
    typedef std::chrono::steady_clock::rep          rep;
    typedef std::chrono::steady_clock::period       period;
    typedef std::chrono::steady_clock::duration     duration;
    typedef std::chrono::steady_clock::time_point   time_point;

    static constexpr bool is_steady = true;

    static time_point now() noexcept
    {
        return std::chrono::steady_clock::now();
    }
};

}}

#endif //DIPLOMA_GC_CLOCK_HPP
