#ifndef DIPLOMA_GC_PAUSE_H
#define DIPLOMA_GC_PAUSE_H

#include <functional>
#include <string>
#include <exception>
#include <signal.h>

#include "thread_list.h"
#include "signal_lock.h"

namespace precisegc { namespace details {

void gc_pause();
void gc_resume();

// this exception is thrown when thread previously called gc_pause_lock.lock() is calling gc_pause()
class gc_pause_disabled_exception: public std::exception
{
public:
    gc_pause_disabled_exception();
    const char* what() const noexcept override;
private:
    std::string m_msg;
};

class gc_pause_lock: public signal_lock_base<gc_pause_lock>, public noncopyable
{
public:
    gc_pause_lock() = default;
    gc_pause_lock(gc_pause_lock&&) = default;
    gc_pause_lock& operator=(gc_pause_lock&&) = default;

    void lock() noexcept;
    void unlock() noexcept;

    friend class signal_lock_base<gc_pause_lock>;
private:
    static sigset_t get_sigset() noexcept;
};

typedef std::function<void(void)> pause_handler_t;

void set_gc_pause_handler(const pause_handler_t& pause_handler);
pause_handler_t get_gc_pause_handler();

class pause_handler_setter
{
public:
    pause_handler_setter(const pause_handler_t& pause_handler)
            : m_pause_handler(get_gc_pause_handler())
    {
        set_gc_pause_handler(pause_handler);
    }

    ~pause_handler_setter()
    {
        set_gc_pause_handler(m_pause_handler);
    }
private:
    pause_handler_t m_pause_handler;
};

}}

#endif //DIPLOMA_GC_PAUSE_H
