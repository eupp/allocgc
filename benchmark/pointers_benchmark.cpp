#include <nonius/nonius.h++>

#include <vector>
#include <cstdlib>
#include <memory>

#include "libprecisegc/libprecisegc.h"
#include "libprecisegc/details/allocators/index_tree.h"

using namespace precisegc;
using namespace precisegc::details;
using namespace precisegc::details::allocators;

typedef size_t obj_type;

NONIUS_BENCHMARK("new", [](nonius::chronometer meter)
{
    std::vector<obj_type*> ptrs(meter.runs());
    meter.measure([&ptrs] (size_t i) {
        obj_type* obj = new obj_type();
        ptrs[i] = obj;
        return obj;
    });
    for (auto ptr: ptrs) {
        delete ptr;
    }
});

NONIUS_BENCHMARK("make_shared", [](nonius::chronometer meter)
{
    std::vector<std::shared_ptr<obj_type>> ptrs(meter.runs());
    meter.measure([&ptrs] (size_t i) {
        auto obj = std::make_shared<obj_type>();
        ptrs[i] = obj;
        return obj;
    });
});

NONIUS_BENCHMARK("gc_new", [](nonius::chronometer meter)
{
    gc_init();
    meter.measure([] {
        gc_ptr<obj_type> p = gc_new<obj_type>();
        return p;
    });
});

namespace {
static const size_t PTR_CNT = 10;
}

NONIUS_BENCHMARK("raw_ptr_constructor", [](nonius::chronometer meter)
{
    std::vector<nonius::storage_for<obj_type*>> storage(meter.runs());
    meter.measure([&storage] (size_t i) {
        storage[i].construct();
    });
});

NONIUS_BENCHMARK("shared_ptr_constructor", [](nonius::chronometer meter)
{
    std::vector<nonius::storage_for<std::shared_ptr<obj_type>>> storage(meter.runs());
    meter.measure([&storage] (size_t i) {
        storage[i].construct();
    });
});

NONIUS_BENCHMARK("gc_ptr_constructor", [](nonius::chronometer meter)
{
    std::vector<nonius::storage_for<gc_ptr<obj_type>>> storage(meter.runs());
    meter.measure([&storage] (size_t i) {
        storage[i].construct();
    });
});

NONIUS_BENCHMARK("raw_ptr_assign", [](nonius::chronometer meter)
{
    std::vector<obj_type*> ptrs(meter.runs());
    obj_type* p;
    meter.measure([&ptrs, p] (size_t i) {
        ptrs[i] = p;
    });
});

NONIUS_BENCHMARK("shared_ptr_assign", [](nonius::chronometer meter)
{
    std::vector<std::shared_ptr<obj_type>> ptrs(meter.runs());
    auto p = std::make_shared<obj_type>(42);
    meter.measure([&ptrs, p] (size_t i) {
        ptrs[i] = p;
    });
});

NONIUS_BENCHMARK("gc_ptr_assign", [](nonius::chronometer meter)
{
    std::vector<gc_ptr<obj_type>> ptrs(meter.runs());
    auto p = gc_new<obj_type>(42);
    meter.measure([&ptrs, p] (size_t i) {
        ptrs[i] = p;
    });
});

NONIUS_BENCHMARK("raw_ptr_deref", [](nonius::chronometer meter)
{
    std::vector<obj_type*> ptrs(meter.runs());
    meter.measure([&ptrs] (size_t i) {
        return *ptrs[i];
    });
});

NONIUS_BENCHMARK("shared_ptr_deref", [](nonius::chronometer meter)
{
    std::vector<std::shared_ptr<obj_type>> ptrs(meter.runs());
    for (auto& ptr: ptrs) {
        ptr = std::make_shared<obj_type>(42);
    }
    meter.measure([&ptrs] (size_t i) {
        return *ptrs[i];
    });
});

NONIUS_BENCHMARK("gc_ptr_deref", [](nonius::chronometer meter)
{
    std::vector<gc_ptr<obj_type>> ptrs(meter.runs());
    for (auto& ptr: ptrs) {
        ptr = gc_new<obj_type>(42);
    }
    meter.measure([&ptrs] (size_t i) {
        gc_pin<obj_type> pin(ptrs[i]);
        return *pin;
    });
});