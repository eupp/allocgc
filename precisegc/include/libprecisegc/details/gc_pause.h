#ifndef DIPLOMA_GC_PAUSE_H
#define DIPLOMA_GC_PAUSE_H

#include <functional>
#include <string>
#include <exception>
#include <signal.h>

#include "thread_list.h"
#include "sigmask_signal_lock.h"

namespace precisegc { namespace details {

void gc_pause_init();

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

class flag_gc_signal_lock
{
public:
    static int lock() noexcept;
    static int unlock() noexcept;
    static bool is_locked() noexcept;
    static void set_pending() noexcept;
private:
    static thread_local volatile sig_atomic_t depth;
    static thread_local volatile sig_atomic_t signal_pending_flag;
};

class gc_pause_lock: public noncopyable, public nonmovable
{
public:
    gc_pause_lock() = default;
    void lock() noexcept;
    void unlock() noexcept;
private:
    typedef flag_gc_signal_lock siglock;
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
