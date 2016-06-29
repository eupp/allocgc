#include <nonius/nonius.h++>

#include <vector>

#include <boost/lockfree/queue.hpp>
#include <boost/lockfree/spsc_queue.hpp>

static const size_t QUEUE_SIZE = 16384;

NONIUS_BENCHMARK("lock_free.dyn_queue_push", [](nonius::chronometer meter)
{
    boost::lockfree::queue<size_t, boost::lockfree::fixed_sized<false>> queue;
    meter.measure([&queue] () {
        return queue.push(1);
    });
});

NONIUS_BENCHMARK("lock_free.dyn_queue_pop", [](nonius::chronometer meter)
{
    boost::lockfree::queue<size_t, boost::lockfree::fixed_sized<false>> queue;
    for (size_t i = 0 ; i < meter.runs(); ++i) {
        queue.push(i);
    }
    meter.measure([&queue] () {
        size_t val;
        return queue.pop(val);
    });
});

NONIUS_BENCHMARK("lock_free.fixed_size_queue_push", [](nonius::chronometer meter)
{
    boost::lockfree::queue<size_t, boost::lockfree::fixed_sized<true>> queue(QUEUE_SIZE);
    meter.measure([&queue] () {
        return queue.push(1);
    });
});

NONIUS_BENCHMARK("lock_free.fixed_size_queue_pop", [](nonius::chronometer meter)
{
    boost::lockfree::queue<size_t, boost::lockfree::fixed_sized<true>> queue(QUEUE_SIZE);
    for (size_t i = 0 ; i < meter.runs(); ++i) {
        queue.push(i);
    }
    meter.measure([&queue] () {
        size_t val;
        return queue.pop(val);
    });
});

NONIUS_BENCHMARK("lock_free.spsc_queue_push", [](nonius::chronometer meter)
{
    boost::lockfree::spsc_queue<size_t, boost::lockfree::capacity<QUEUE_SIZE>> queue;
    meter.measure([&queue] () {
        return queue.push(1);
    });
});

NONIUS_BENCHMARK("lock_free.spsc_queue_pop", [](nonius::chronometer meter)
{
    boost::lockfree::spsc_queue<size_t, boost::lockfree::capacity<QUEUE_SIZE>> queue;
    for (size_t i = 0 ; i < meter.runs(); ++i) {
        queue.push(i);
    }
    meter.measure([&queue] () {
        size_t val;
        return queue.pop(val);
    });
});

NONIUS_BENCHMARK("lock_free.unsync_push", [](nonius::chronometer meter)
{
    std::vector<size_t> queue;
    queue.reserve(QUEUE_SIZE);
    meter.measure([&queue] () {
        return queue.push_back(1);
    });
});

NONIUS_BENCHMARK("lock_free.unsync_pop", [](nonius::chronometer meter)
{
    std::vector<size_t> queue;
    queue.reserve(QUEUE_SIZE);
    for (size_t i = 0 ; i < meter.runs(); ++i) {
        queue.push_back(i);
    }
    meter.measure([&queue] () {
        return queue.pop_back();
    });
});
