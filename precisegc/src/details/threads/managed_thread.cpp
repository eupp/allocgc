#include <libprecisegc/details/threads/managed_thread.hpp>

namespace precisegc { namespace details { namespace threads {

std::unique_ptr<managed_thread> managed_thread::main_thread_ptr{};

}}}

