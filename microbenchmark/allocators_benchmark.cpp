#include <nonius/nonius.h++>

#include <vector>

#include <libprecisegc/details/allocators/default_allocator.hpp>
#include <libprecisegc/details/allocators/intrusive_list_allocator.hpp>
#include <libprecisegc/details/allocators/freelist_pool_chunk.hpp>

using namespace precisegc::details;
using namespace precisegc::details::allocators;

static const size_t OBJ_SIZE = 32;

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

namespace {
typedef intrusive_list_pool_allocator<
        freelist_pool_chunk, allocators::default_allocator
    > intrusive_list_allocator_t;
}

NONIUS_BENCHMARK("allocators.intrusive_list_pool_allocator.allocate", [](nonius::chronometer meter)
{
    intrusive_list_allocator_t allocator;
    std::vector<byte*> ps(meter.runs());
    meter.measure([&allocator, &ps] (size_t i) {
        ps[i] = allocator.allocate(OBJ_SIZE);
    });
    for (auto p: ps) {
        allocator.deallocate(p, OBJ_SIZE);
    }
});

NONIUS_BENCHMARK("allocators.intrusive_list_pool_allocator.deallocate", [](nonius::chronometer meter)
{
    intrusive_list_allocator_t allocator;
    std::vector<byte*> ps(meter.runs());
    for (auto& p: ps) {
        p = allocator.allocate(OBJ_SIZE);
    }
    meter.measure([&allocator, &ps] (size_t i) {
        allocator.deallocate(ps[i], OBJ_SIZE);
    });
});
