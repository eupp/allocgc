#ifndef DIPLOMA_GC_PAUSE_H
#define DIPLOMA_GC_PAUSE_H

#include <functional>
#include <string>
#include <exception>

namespace precisegc { namespace details {

// this exception is thrown when thread previously called disable_gc_pause is calling gc_pause
class gc_pause_disabled_exception: public std::exception
{
public:
    gc_pause_disabled_exception();
    const char* what() const noexcept override;
private:
    std::string m_msg;
};

// enable/disable interruption of current thread by gc_pause
void enable_gc_pause();
void disable_gc_pause();

class gc_pause_lock
{
public:
    gc_pause_lock()
    {
        disable_gc_pause();
    }

    ~gc_pause_lock()
    {
        enable_gc_pause();
    }
};

void gc_pause();
void gc_resume();

typedef std::function<void(void)> pause_handler_t;

void set_gc_pause_handler(const pause_handler_t& pause_handler);
pause_handler_t get_gc_pause_handler();

}}

#endif //DIPLOMA_GC_PAUSE_H
