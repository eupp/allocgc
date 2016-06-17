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
