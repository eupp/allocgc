#include <libprecisegc/details/threads/approx_stack_map.hpp>

namespace precisegc { namespace details { namespace threads {

approx_stack_map::approx_stack_map()
{ }

void approx_stack_map::register_root(gc_handle* root)
{ }

void approx_stack_map::deregister_root(gc_handle* root)
{ }

bool approx_stack_map::contains(const gc_handle* ptr)
{ }

size_t approx_stack_map::count() const
{ }

}}}