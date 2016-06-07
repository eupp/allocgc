#include "logging.h"

#include <cassert>

namespace precisegc { namespace details {

const char* logging::prefix = "precisegc-";
std::unique_ptr<logging::logger> logging::logger_ = nullptr;
std::unique_ptr<threads::ass_mutex> logging::mutex_ = nullptr;
gc_loglevel logging::loglevel_ = gc_loglevel::OFF;

void logging::init(std::ostream& stream, gc_loglevel lv)
{
    logger_.reset(new logger(stream));
    mutex_.reset(new threads::ass_mutex());
    loglevel_ = lv;
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

logging::log_line logging::log(gc_loglevel lv)
{
    assert(logger_);
    return log_line(lv);
}

logging::logger::logger(std::ostream& stream)
    : m_stream(stream.rdbuf())
{
    m_stream.rdbuf()->pubsetbuf(0, 0);
}

logging::log_line::log_line(gc_loglevel lv)
    : m_active(lv >= loglevel_)
{
    if (m_active) {
        mutex_->lock();

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
                lv_str = "DEBUG";
                break;
            case gc_loglevel::INFO:
                lv_str = "INFO";
                break;
            case gc_loglevel::WARNING:
                lv_str = "WARNING";
                break;
            case gc_loglevel::ERROR:
                lv_str = "ERROR";
                break;
        }
        (*this) << time_buffer << prefix << lv_str << ": ";
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