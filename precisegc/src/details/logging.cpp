#include <libprecisegc/details/logging.hpp>

#include <cassert>

#include <libprecisegc/details/threads/posix_thread.hpp>

namespace precisegc { namespace details {

const char* logging::prefix_ = "precisegc-";

void logging::init(std::ostream& stream, gc_loglevel lv)
{
    get_logger() = boost::in_place(stream, lv);
}

logging::log_line logging::debug()
{
    return log(gc_loglevel::DEBUG);
}

logging::log_line logging::info()
{
    return log(gc_loglevel::INFO);
}

logging::log_line logging::warning()
{
    return log(gc_loglevel::WARNING);
}

logging::log_line logging::error()
{
    return log(gc_loglevel::ERROR);
}

void logging::touch()
{
    get_logger();
}

boost::optional<logging::logger>& logging::get_logger()
{
    static boost::optional<logging::logger> lg;
    return lg;
}

logging::log_line logging::log(gc_loglevel lv)
{
    assert(get_logger());
    return log_line(lv);
}

logging::logger::logger(const std::ostream& stream, gc_loglevel lv)
    : m_stream(stream.rdbuf())
    , m_loglevel(lv)
{
    m_stream.rdbuf()->pubsetbuf(0, 0);
}

logging::log_line::log_line(gc_loglevel lv)
    : m_lock(get_logger()->m_mutex, std::defer_lock)
{
    if (lv >= get_logger()->m_loglevel) {
        m_lock.lock();

        time_t rawtime;
        struct tm * timeinfo;
        static const size_t BUF_SIZE = 64;
        char time_buffer[BUF_SIZE] = {0};
        time (&rawtime);
        timeinfo = localtime(&rawtime);
        strftime(time_buffer, BUF_SIZE, "%d-%m-%Y %I:%M:%S-", timeinfo);

        const char* lv_str = nullptr;
        switch (lv)
        {
            case gc_loglevel::DEBUG:
                lv_str = "DEBUG:  ";
                break;
            case gc_loglevel::INFO:
                lv_str = "INFO:   ";
                break;
            case gc_loglevel::WARNING:
                lv_str = "WARNING: ";
                break;
            case gc_loglevel::ERROR:
                lv_str = "ERROR:   ";
                break;
        }
        (*this) << time_buffer << prefix_ << threads::this_thread_native_handle() << "-" << lv_str;
    }
}

logging::log_line::~log_line()
{
    if (m_lock.owns_lock()) {
        (*this) << std::endl<char, std::char_traits<char>>;
    }
}

}}