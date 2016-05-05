#include <nonius/nonius.h++>

#include "libprecisegc/details/sigmask_signal_lock.h"
#include "libprecisegc/details/gc_pause.h"
#include "libprecisegc/details/threads/pending_call.hpp"

using namespace precisegc::details;
using namespace precisegc::details::threads;

NONIUS_BENCHMARK("sigmask_signal_lock", [](nonius::chronometer meter)
{
    meter.measure([] {
        sigmask_gc_signal_lock::lock();
        return sigmask_gc_signal_lock::unlock();
    });
});

NONIUS_BENCHMARK("flag_signal_lock", [](nonius::chronometer meter)
{
    meter.measure([] {
        flag_gc_signal_lock::lock();
        return flag_gc_signal_lock::unlock();
    });
});

static void f() {}

NONIUS_BENCHMARK("pending_call_lock", [](nonius::chronometer meter)
{
    pending_call pcall(f);
    meter.measure([&pcall] {
        pcall.lock();
        pcall.unlock();
    });
});
