#include "logging.h"

#include <cassert>

namespace precisegc { namespace details {

const char* logging::prefix = "precisegc-";
std::unique_ptr<logging::logger> logging::logger_ = nullptr;
std::unique_ptr<gc_signal_safe_mutex> logging::mutex_ = nullptr;

void logging::init(std::ostream& stream)
{
    logger_.reset(new logger(stream));
    mutex_.reset(new gc_signal_safe_mutex());
}

logging::locking_wrapper logging::debug()
{
    return log("DEBUG");
}

logging::locking_wrapper logging::info()
{
    return log("INFO");
}

logging::locking_wrapper logging::warning()
{
    return log("WARNING");
}

logging::locking_wrapper logging::error()
{
    return log("ERROR");
}

logging::locking_wrapper logging::log(const char* loglevel)
{
    assert(logger_);
    locking_wrapper wrapper;
    wrapper << prefix << loglevel << ": ";
    return wrapper;
}

logging::logger::logger(std::ostream& stream)
    : m_stream(stream.rdbuf())
{
    m_stream.rdbuf()->pubsetbuf(0, 0);
}

logging::locking_wrapper::locking_wrapper()
    : m_active(true)
{
    mutex_->lock();
}

logging::locking_wrapper::~locking_wrapper()
{
    if (m_active) {
        operator<<(std::endl<char, std::char_traits<char>>);
        mutex_->unlock();
    }
}

logging::locking_wrapper::locking_wrapper(logging::locking_wrapper&& wrapper)
    : m_active(true)
{
    wrapper.m_active = false;
}

}}