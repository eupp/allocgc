#ifndef DIPLOMA_LOGGING_H
#define DIPLOMA_LOGGING_H

#include <iostream>
#include <memory>

#include "signal_safe_sync.h"
#include "util.h"

namespace precisegc { namespace details {

class logging
{
    class locking_wrapper;
public:

    static void init(std::ostream& stream);
    static locking_wrapper debug();
    static locking_wrapper info();
    static locking_wrapper warning();
    static locking_wrapper error();

private:
    class logger: public noncopyable, public nonmovable
    {
    public:
        logger(std::ostream& stream);

        template <typename T>
        logger& operator<<(const T& x)
        {
            m_stream << x;
            return *this;
        }
    private:
        std::ostream m_stream;
    };

    class locking_wrapper: public noncopyable
    {
    public:
        locking_wrapper();
        ~locking_wrapper();

        locking_wrapper(locking_wrapper&&) = default;
        locking_wrapper& operator=(locking_wrapper&&) = delete;

        template <typename T>
        locking_wrapper& operator<<(const T& x)
        {
            (*logger_) << x;
            return *this;
        }
    private:
        gc_signal_safe_mutex m_mutex;
    };

    static locking_wrapper log(const char* loglevel);

    static const char* prefix;
    // to do: switch to optional<logger>
    static std::unique_ptr<logger> logger_;
};

}}

#endif //DIPLOMA_LOGGING_H
