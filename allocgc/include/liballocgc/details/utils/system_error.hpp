#ifndef ALLOCGC_CHECK_SYS_RC_HPP
#define ALLOCGC_CHECK_SYS_RC_HPP

#include <cerrno>
#include <system_error>

#include <liballocgc/details/logging.hpp>

namespace allocgc { namespace details { namespace utils {

inline void throw_system_error(long long int rc, const char* errmsg)
{
    if (rc < 0) {
        auto exc = std::system_error(errno, std::system_category(), errmsg);
        logging::error() << exc.what();
        throw exc;
    }
}

inline void log_system_error(long long int rc, const char* errmsg)
{
    if (rc < 0) {
        auto exc = std::system_error(errno, std::system_category(), errmsg);
        logging::error() << exc.what();
    }
}

}}}

#endif //ALLOCGC_CHECK_SYS_RC_HPP
