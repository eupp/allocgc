#include <benchmark/benchmark.h>

#include <memory>
#include <cstddef>

#include "liballocgc/gc.hpp"
#include "liballocgc/details/gc_facade.hpp"
#include "liballocgc/details/collectors/gc_core.hpp"

#include "../../common/timer.hpp"

using namespace allocgc;
using namespace allocgc::details;
using namespace allocgc::details::collectors;

static void malloc_benchmark(benchmark::State& state) {
    size_t size = state.range(0);
    while (state.KeepRunning()) {
        void* p = malloc(size);
        benchmark::DoNotOptimize(p);
        state.PauseTiming();
        free(p);
        state.ResumeTiming();
    }
}
BENCHMARK(malloc_benchmark)->RangeMultiplier(2)->Range(32, 1<<16);

namespace {
struct test_type
{ };

const gc_type_meta* type_meta = gc_type_meta_factory<test_type>::create();
}

static void gc_alloc_benchmark(benchmark::State& state) {
    typedef gc_facade<gc_serial> gc_facade;

    gc_buf buf;
    size_t cnt = 0;
    gc_alloc::request rqst(64, 1, type_meta, &buf);
    while (state.KeepRunning()) {
        timer tm;
        ++cnt;
        gc_alloc::response rsp = gc_facade::allocate(rqst);
        auto elapsed = tm.elapsed<std::chrono::duration<double>>();

        gc_facade::commit(rsp);

        state.SetIterationTime(elapsed);
    }
    state.counters["cnt"] = cnt;
}
//BENCHMARK(gc_alloc_benchmark);

