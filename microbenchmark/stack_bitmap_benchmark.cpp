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
    meter.measure([&stack_map] {
        gc_word h;
        stack_map.register_root(&h);
    });
});

NONIUS_BENCHMARK("stack_bitmap.deregister_root", [](nonius::chronometer meter)
{
    stack_bitmap stack_map(frame_address());
    meter.measure([&stack_map] {
        gc_word h;
        stack_map.deregister_root(&h);
    });
});


