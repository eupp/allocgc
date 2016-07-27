#include <libprecisegc/details/threads/tlab.hpp>

namespace precisegc { namespace details { namespace threads {

tlab tlab::dead_tlab_{};
std::mutex tlab::migrate_mutex{};

}}}