#include <nonius/nonius.h++>

#include <cstdlib>
#include <vector>

#include <libprecisegc/details/utils/math.hpp>

#include "deoptimize.hpp"

using namespace precisegc::details;

namespace {
typedef unsigned long long ull;
}

NONIUS_BENCHMARK("math.msb", [](nonius::chronometer meter)
{
    srand(time(0));
    std::vector<ull> rs(meter.runs());
    for (auto& r : rs) {
        r = ((ull) rand() << 32) + rand();
    }
    meter.measure([&rs] (size_t i) {
        volatile int k = msb(rs[i]);
        return k;
    });
});

