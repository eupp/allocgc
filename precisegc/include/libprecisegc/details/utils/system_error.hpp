#ifndef DIPLOMA_CHECK_SYS_RC_HPP
#define DIPLOMA_CHECK_SYS_RC_HPP

#include <cerrno>
#include <system_error>

#include <libprecisegc/details/logging.h>

namespace precisegc { namespace details { namespace utils {

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

#endif //DIPLOMA_CHECK_SYS_RC_HPP
