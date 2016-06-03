#include <nonius/nonius.h++>

#include "libprecisegc/details/threads/pending_call.hpp"

using namespace precisegc::details;
using namespace precisegc::details::threads;

static void f() {}

//NONIUS_BENCHMARK("pending_call_lock", [](nonius::chronometer meter)
//{
//    pending_call pcall(f);
//    meter.measure([&pcall] {
//        pcall.lock();
//        pcall.unlock();
//    });
//});
