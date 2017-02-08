#include <nonius/nonius.h++>

#include <vector>

#include <libprecisegc/details/collectors/pin_set.hpp>
#include <libprecisegc/details/gc_unsafe_scope.hpp>

#include "deoptimize.hpp"

using namespace precisegc::details;
using namespace precisegc::details::threads;

NONIUS_BENCHMARK("pin_set.insert", [](nonius::chronometer meter)
{
    pin_set pins;
    meter.measure([&pins] {
        byte* p;
        pins.insert(p);
    });
});

NONIUS_BENCHMARK("pin_set.remove", [](nonius::chronometer meter)
{
    pin_set pins;
    std::vector<byte*> ps(meter.runs());
    size_t size = ps.size();
    for (auto p: ps) {
        pins.insert(p);
    }
    meter.measure([&pins, &ps, size] (size_t i) {
        pins.remove(ps[size - 1 - i]);
        escape(&pins);
    });
});
