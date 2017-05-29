#include <liballocgc/details/logging.hpp>

#include <cassert>

#include <liballocgc/gc_common.hpp>
#include <liballocgc/details/threads/posix_thread.hpp>

namespace allocgc { namespace details {

const char* logging::prefix_ = "allocgc-";

void logging::set_loglevel(gc_loglevel loglevel)
{
    get_logger()->set_loglevel(loglevel);
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

std::unique_ptr<logging::logger>& logging::get_logger()
{
    static std::unique_ptr<logging::logger> lg{new logger(std::clog, gc_loglevel::SILENT)};
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

void logging::logger::set_loglevel(gc_loglevel loglevel)
{
    m_loglevel = loglevel;
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
                lv_str = "DEBUG: ";
                break;
            case gc_loglevel::INFO:
                lv_str = "INFO: ";
                break;
            case gc_loglevel::WARNING:
                lv_str = "WARNING: ";
                break;
            case gc_loglevel::ERROR:
                lv_str = "ERROR: ";
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