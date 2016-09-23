#include <nonius/nonius.h++>

#include <vector>

#include <libprecisegc/details/threads/approx_stack_map.hpp>
#include <libprecisegc/details/threads/return_address.hpp>

#include "deoptimize.hpp"

using namespace precisegc::details;
using namespace precisegc::details::threads;

NONIUS_BENCHMARK("approx_stack_map.register_root", [](nonius::chronometer meter)
{
    approx_stack_map stack_map(frame_address());
    meter.measure([&stack_map] {
        gc_handle h;
        stack_map.register_root(&h);
    });
});

NONIUS_BENCHMARK("approx_stack_map.deregister_root", [](nonius::chronometer meter)
{
    approx_stack_map stack_map(frame_address());
    meter.measure([&stack_map] {
        gc_handle h;
        stack_map.deregister_root(&h);
    });
});


