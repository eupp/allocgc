#ifndef DIPLOMA_LOGGING_H
#define DIPLOMA_LOGGING_H

#include <iostream>
#include <memory>

#include <libprecisegc/details/threads/ass_sync.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/gc_options.hpp>

namespace precisegc { namespace details {

class logging
{
    class log_line;
public:
    static void init(std::ostream& stream, gc_loglevel lv);
    static log_line debug();
    static log_line info();
    static log_line warning();
    static log_line error();
private:
    class logger: public utils::noncopyable, public utils::nonmovable
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

    class log_line: public utils::noncopyable
    {
    public:
        explicit log_line(gc_loglevel lv);
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

    static log_line log(gc_loglevel lv);

    static const char* prefix;
    // to do: switch to optional<logger>
    static std::unique_ptr<logger> logger_;
    // to do: switch to optional<gc_signal_safe_mutex>
    static std::unique_ptr<threads::ass_mutex> mutex_;
    static gc_loglevel loglevel_;
};

}}

#endif //DIPLOMA_LOGGING_H
