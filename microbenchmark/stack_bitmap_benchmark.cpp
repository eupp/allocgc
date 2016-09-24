#include <nonius/nonius.h++>

#include <vector>

#include <libprecisegc/details/threads/stack_bitmap.hpp>
#include <libprecisegc/details/threads/return_address.hpp>

#include "deoptimize.hpp"

using namespace precisegc::details;
using namespace precisegc::details::threads;

NONIUS_BENCHMARK("stack_bitmap.register_root", [](nonius::chronometer meter)
{
    stack_bitmap stack_map(frame_address());
    meter.measure([&pin_set] {
        gc_handle h;
        pin_set.register_root(&h);
    });
});

NONIUS_BENCHMARK("stack_bitmap.deregister_root", [](nonius::chronometer meter)
{
    stack_bitmap stack_map(frame_address());
    meter.measure([&pin_set] {
        gc_handle h;
        pin_set.deregister_root(&h);
    });
});


