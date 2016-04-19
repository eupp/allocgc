#include <nonius/nonius.h++>

#include <atomic>
#include <vector>
#include <mutex>
#include <queue>
#include <boost/lockfree/queue.hpp>
#include <boost/lockfree/spsc_queue.hpp>

#include "libprecisegc/details/gc_unsafe_scope.h"

using namespace precisegc::details;

//NONIUS_BENCHMARK("plain assign", [](nonius::chronometer meter)
//{
//    std::vector<void*> ps1(meter.runs());
//    std::vector<void*> ps2(meter.runs());
//    meter.measure([&ps1, &ps2] (size_t i) {
//        ps1[i] = ps2[i];
//    });
//});
//
//NONIUS_BENCHMARK("relaxed assign", [](nonius::chronometer meter)
//{
//    std::vector<std::atomic<void*>> ps1(meter.runs());
//    std::vector<std::atomic<void*>> ps2(meter.runs());
//    meter.measure([&ps1, &ps2] (size_t i) {
//        ps1[i].store(ps2[i].load(std::memory_order_relaxed), std::memory_order_relaxed);
//    });
//});
//
NONIUS_BENCHMARK("acq_rel assign", [](nonius::chronometer meter)
{
    std::vector<std::atomic<void*>> ps1(meter.runs());
    std::vector<std::atomic<void*>> ps2(meter.runs());
    meter.measure([&ps1, &ps2] (size_t i) {
        ps1[i].store(ps2[i].load(std::memory_order_acquire), std::memory_order_release);
    });
});
//
//NONIUS_BENCHMARK("sequential assign", [](nonius::chronometer meter)
//{
//    std::vector<std::atomic<void*>> ps1(meter.runs());
//    std::vector<std::atomic<void*>> ps2(meter.runs());
//    meter.measure([&ps1, &ps2] (size_t i) {
//        ps1[i].store(ps2[i].load());
//    });
//});

NONIUS_BENCHMARK("unsafe scope", [](nonius::chronometer meter)
{
    meter.measure([] () {
        gc_unsafe_scope unsafe_scope;
    });
});

NONIUS_BENCHMARK("mutex queue", [](nonius::chronometer meter)
{
    std::mutex mutex;
    std::queue<void*> queue;
    std::vector<void*> ps(meter.runs());
    meter.measure([&queue, &ps, &mutex] (size_t i) {
        std::lock_guard<std::mutex> lock(mutex);
        return queue.push(ps[i]);
    });
});

NONIUS_BENCHMARK("lockfree queue", [](nonius::chronometer meter)
{
    boost::lockfree::queue<void*, boost::lockfree::fixed_sized<false>> queue(0);
    std::vector<void*> ps(meter.runs());
    meter.measure([&queue, &ps] (size_t i) {
        return queue.push(ps[i]);
    });
});

NONIUS_BENCHMARK("spsc queue", [](nonius::chronometer meter)
{
    boost::lockfree::spsc_queue<void*, boost::lockfree::fixed_sized<false>> queue(0);
    std::vector<void*> ps(meter.runs());
    meter.measure([&queue, &ps] (size_t i) {
        return queue.push(ps[i]);
    });
});