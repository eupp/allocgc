#ifndef DIPLOMA_TIMER_HPP
#define DIPLOMA_TIMER_HPP

#include <chrono>

class timer
{
    typedef std::chrono::steady_clock clock_t;
public:
    timer()
        : m_tm(clock_t::now())
    {}

    timer(const timer&) = delete;
    timer(timer&&) = delete;

    timer& operator=(const timer&) = delete;
    timer& operator=(timer&&) = delete;

    template <typename Duration>
    Duration elapsed() const
    {
        return std::chrono::duration_cast<Duration>(clock_t::now() - m_tm);
    }
private:
    clock_t::time_point m_tm;
};

#endif //DIPLOMA_TIMER_HPP
