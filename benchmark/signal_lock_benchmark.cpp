#include <nonius/nonius.h++>

#include "libprecisegc/details/sigmask_signal_lock.h"

using namespace precisegc::details;

NONIUS_BENCHMARK("sigmask_signal_lock", [](nonius::chronometer meter)
{
    meter.measure([] {
        sigmask_gc_signal_lock::lock();
        sigmask_gc_signal_lock::unlock();
    });
});



//NONIUS_BENCHMARK("sigmask_signal_unlock", [](nonius::chronometer meter)
//{
//    sigmask_gc_signal_lock::lock();
//    meter.measure([] { return sigmask_gc_signal_lock::unlock(); });
//});
