#ifndef ALLOCGC_TIMER_HPP
#define ALLOCGC_TIMER_HPP

#include <chrono>

class timer
{
    typedef std::chrono::high_resolution_clock clock_t;
public:
    timer()
        : m_tm(clock_t::now())
    {}

    timer(const timer&) = delete;
    timer(timer&&) = delete;

    timer& operator=(const timer&) = delete;
    timer& operator=(timer&&) = delete;

    void reset()
    {
        m_tm = clock_t::now();
    }

    template <typename Duration>
    typename Duration::rep elapsed() const
    {
        return std::chrono::duration_cast<Duration>(clock_t::now() - m_tm).count();
    }
private:
    clock_t::time_point m_tm;
};

#endif //ALLOCGC_TIMER_HPP
