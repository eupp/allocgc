#ifndef ALLOCGC_LOGGING_H
#define ALLOCGC_LOGGING_H

#include <iostream>
#include <memory>
#include <mutex>

#include <liballocgc/gc_common.hpp>
#include <liballocgc/details/utils/utility.hpp>

namespace allocgc { namespace details {

class logging
{
    class log_line;
public:
    static void set_loglevel(gc_loglevel loglevel);

    static log_line debug();
    static log_line info();
    static log_line warning();
    static log_line error();
private:
    typedef std::mutex mutex_t;

    class logger : private utils::noncopyable, private utils::nonmovable
    {
    public:
        logger(const std::ostream& stream, gc_loglevel lv);

        void set_loglevel(gc_loglevel loglevel);

        template <typename T>
        logger& operator<<(const T& x)
        {
            m_stream << x;
            return *this;
        }

        friend class log_line;
    private:
        std::ostream m_stream;
        gc_loglevel m_loglevel;
        mutex_t m_mutex;
    };

    class log_line: private utils::noncopyable
    {
    public:
        explicit log_line(gc_loglevel lv);

        ~log_line();

        log_line(log_line&&) = default;
        log_line& operator=(log_line&&) = delete;

        template <typename T>
        log_line& operator<<(const T& x)
        {
            if (m_lock.owns_lock()) {
                (*get_logger()) << x;
            }
            return *this;
        }
    private:
        std::unique_lock<mutex_t> m_lock;
    };

    static std::unique_ptr<logger>& get_logger();
    static log_line log(gc_loglevel lv);

    static const char* prefix_;
};

}}

#endif //ALLOCGC_LOGGING_H
