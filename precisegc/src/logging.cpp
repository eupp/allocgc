#include "logging.h"

#include <cassert>

namespace precisegc { namespace details {

const char* logging::prefix = "precisegc-";
std::unique_ptr<logging::logger> logging::logger_ = nullptr;
std::unique_ptr<gc_signal_safe_mutex> logging::mutex_ = nullptr;
logging::loglevel logging::loglevel_ = logging::loglevel::OFF;

void logging::init(std::ostream& stream, loglevel lv)
{
    logger_.reset(new logger(stream));
    mutex_.reset(new gc_signal_safe_mutex());
    loglevel_ = lv;
}

logging::log_line logging::debug()
{
    return log(loglevel::DEBUG);
}

logging::log_line logging::info()
{
    return log(loglevel::INFO);
}

logging::log_line logging::warning()
{
    return log(loglevel::WARNING);
}

logging::log_line logging::error()
{
    return log(loglevel::ERROR);
}

logging::log_line logging::log(loglevel lv)
{
    assert(logger_);
    return log_line(lv);
}

logging::logger::logger(std::ostream& stream)
    : m_stream(stream.rdbuf())
{
    m_stream.rdbuf()->pubsetbuf(0, 0);
}

logging::log_line::log_line(loglevel lv)
    : m_active(lv >= loglevel_)
{
    if (m_active) {
        mutex_->lock();
        const char* lv_str = nullptr;
        switch (lv)
        {
            case loglevel::DEBUG:
                lv_str = "DEBUG";
                break;
            case loglevel::INFO:
                lv_str = "INFO";
                break;
            case loglevel::WARNING:
                lv_str = "WARNING";
                break;
            case loglevel::ERROR:
                lv_str = "ERROR";
                break;
        }
        (*this) << prefix << lv_str << ": ";
    }
}

logging::log_line::~log_line()
{
    if (m_active) {
        (*this) << std::endl<char, std::char_traits<char>>;
        mutex_->unlock();
    }
}

logging::log_line::log_line(logging::log_line&& other)
    : m_active(true)
{
    other.m_active = false;
}

}}