#ifndef DIPLOMA_LOGGING_H
#define DIPLOMA_LOGGING_H

#include <iostream>
#include <memory>

#include "signal_safe_sync.h"
#include "util.h"

namespace precisegc { namespace details {

class logging
{
    class log_line;
public:

    enum class loglevel {
        DEBUG,
        INFO,
        WARNING,
        ERROR,
        OFF
    };

    static void init(std::ostream& stream, loglevel lv);
    static log_line debug();
    static log_line info();
    static log_line warning();
    static log_line error();

private:
    class logger: public noncopyable, public nonmovable
    {
    public:
        explicit logger(std::ostream& stream);

        template <typename T>
        logger& operator<<(const T& x)
        {
            m_stream << x;
            return *this;
        }
    private:
        std::ostream m_stream;
    };

    class log_line: public noncopyable
    {
    public:
        explicit log_line(loglevel lv);
        ~log_line();

        log_line(log_line&&);
        log_line& operator=(log_line&&) = delete;

        template <typename T>
        log_line& operator<<(const T& x)
        {
            if (m_active) {
                (*logger_) << x;
            }
            return *this;
        }
    private:
        bool m_active;
    };

    static log_line log(loglevel lv);

    static const char* prefix;
    // to do: switch to optional<logger>
    static std::unique_ptr<logger> logger_;
    // to do: switch to optional<gc_signal_safe_mutex>
    static std::unique_ptr<gc_signal_safe_mutex> mutex_;
    static loglevel loglevel_;
};

}}

#endif //DIPLOMA_LOGGING_H
