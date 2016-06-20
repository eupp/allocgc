#include <nonius/nonius.h++>//

#include <vector>

#include <libprecisegc/details/stack_map.hpp>
#include <libprecisegc/details/gc_unsafe_scope.h>

using namespace precisegc::details;

NONIUS_BENCHMARK("stack_map_benchmark.insert", [](nonius::chronometer meter)
{
    stack_map<void*> map;
    meter.measure([&map] {
        map.insert(nullptr);
    });
});

NONIUS_BENCHMARK("stack_map_benchmark.remove", [](nonius::chronometer meter)
{
    stack_map<void*> map;
    std::vector<void*> ps(meter.runs());
    size_t size = ps.size();
    for (auto p: ps) {
        map.insert(p);
    }
    meter.measure([&map, &ps, size] (size_t i) {
        map.remove(ps[size - 1 - i]);
    });
});

NONIUS_BENCHMARK("stack_map_benchmark.experimental_insert", [](nonius::chronometer meter)
{
    std::vector<void*> map;
    map.reserve(256);
    void* p = nullptr;
    uintptr_t mask = 1;
    meter.measure([&map, p, mask] {
        gc_unsafe_scope unsafe_scope;
        map.push_back((void*) ((uintptr_t)p | mask));
    });
});

NONIUS_BENCHMARK("stack_map_benchmark.experimental_remove", [](nonius::chronometer meter)
{
    std::vector<uintptr_t> map;
    std::vector<void*> ps(meter.runs());
    size_t size = ps.size();
    for (auto p: ps) {
        map.push_back((uintptr_t) p);
    }
    uintptr_t mask = (~ (uintptr_t) 0) << 1;
    meter.measure([&map, &ps, size, mask] (size_t i) {
        gc_unsafe_scope unsafe_scope;
        size_t j = size - 1 - i;
        while (map[j] != (uintptr_t) ps[j]) {
            --j;
        }
        map[j] &= mask;
    });
});
