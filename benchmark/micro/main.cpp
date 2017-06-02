#include <benchmark/benchmark.h>

#include "liballocgc/gc.hpp"

int main(int argc, char** argv) {

    allocgc::serial::register_main_thread();
    allocgc::serial::set_heap_limit(512 * 1024 * 1024);

    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
}
