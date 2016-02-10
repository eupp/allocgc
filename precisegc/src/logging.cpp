#include "logging.h"

#include <cassert>

namespace precisegc { namespace details {

const char* logging::prefix = "precisegc-";
std::unique_ptr<logging::logger> logging::logger_ = nullptr;

void logging::init(std::ostream& stream)
{
    logger_.reset(new logger(stream));
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
{}


logging::locking_wrapper::locking_wrapper()
{
    m_mutex.lock();
}

logging::locking_wrapper::~locking_wrapper()
{
    m_mutex.unlock();
}

}}