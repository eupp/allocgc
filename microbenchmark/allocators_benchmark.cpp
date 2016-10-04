#include <nonius/nonius.h++>

#include <vector>

#include <libprecisegc/details/allocators/default_allocator.hpp>
#include <libprecisegc/details/allocators/pool.hpp>
#include <libprecisegc/details/gc_hooks.hpp>

#include "deoptimize.hpp"

using namespace precisegc;
using namespace precisegc::details;
using namespace precisegc::details::allocators;

static const size_t OBJ_SIZE = 32;

namespace {
struct test_type
{
    std::uint8_t data[OBJ_SIZE];
};
}

NONIUS_BENCHMARK("allocators.default_allocator.allocate", [](nonius::chronometer meter)
{
    default_allocator allocator;
    std::vector<byte*> ps(meter.runs());
    meter.measure([&allocator, &ps] (size_t i) {
        ps[i] = allocator.allocate(OBJ_SIZE);
    });
    for (auto p: ps) {
        allocator.deallocate(p, OBJ_SIZE);
    }
});

NONIUS_BENCHMARK("allocators.default_allocator.deallocate", [](nonius::chronometer meter)
{
    default_allocator allocator;
    std::vector<byte*> ps(meter.runs());
    for (auto& p: ps) {
        p = allocator.allocate(OBJ_SIZE);
    }
    meter.measure([&allocator, &ps] (size_t i) {
        allocator.deallocate(ps[i], OBJ_SIZE);
    });
});

NONIUS_BENCHMARK("allocators.pool.allocate", [](nonius::chronometer meter)
{
    pool<test_type, utils::dummy_mutex> allocator();
    meter.measure([&allocator] {
        return allocator.create();
    });
});

NONIUS_BENCHMARK("allocators.pool.deallocate", [](nonius::chronometer meter)
{
    pool<test_type, utils::dummy_mutex> allocator(OBJ_SIZE);
    std::vector<byte*> ps(meter.runs());
    for (auto& p: ps) {
        p = allocator.create();
    }
    meter.measure([&allocator, &ps] (size_t i) {
        allocator.destroy(ps[i]);
    });
});

NONIUS_BENCHMARK("allocators.gc_heap.allocate", [](nonius::chronometer meter)
{
    meter.measure([] {
        return gc_allocate(OBJ_SIZE, 0, nullptr);
    });
});
