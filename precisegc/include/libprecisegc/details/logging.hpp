#ifndef DIPLOMA_LOGGING_H
#define DIPLOMA_LOGGING_H

#include <iostream>
#include <memory>
#include <mutex>

#include <boost/optional.hpp>

#include <libprecisegc/gc_init_options.hpp>
#include <libprecisegc/details/utils/utility.hpp>

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

    // auxiliary method to fix order of objects construction/destruction;
    // it's used in some static/global object constructor to guarantee that logger will be constructed before that object
    // and hence it's safe to use logger inside destructor (because it will be destructed after)
    static void touch();
private:
    typedef std::mutex mutex_t;

    class logger : private utils::noncopyable, private utils::nonmovable
    {
    public:
        logger(const std::ostream& stream, gc_loglevel lv);

        template <typename T>
        logger& operator<<(const T& x)
        {
            m_stream << x;
            return *this;
        }

        friend class log_line;
    private:
        std::ostream m_stream;
        mutex_t m_mutex;
        gc_loglevel m_loglevel;
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

    static boost::optional<logger>& get_logger();
    static log_line log(gc_loglevel lv);

    static const char* prefix_;
};

}}

#endif //DIPLOMA_LOGGING_H
