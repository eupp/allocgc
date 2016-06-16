#include <nonius/nonius.h++>

#include <vector>
#include <cstdlib>
#include <memory>

#include <libprecisegc/libprecisegc.h>
#include <libprecisegc/details/ptrs/gc_new_stack.hpp>

using namespace precisegc;

typedef size_t obj_type;

NONIUS_BENCHMARK("ptrs new", [](nonius::chronometer meter)
{
    std::vector<obj_type*> ptrs(meter.runs());
    meter.measure([&ptrs] (size_t i) {
        ptrs[i] = new obj_type;
    });
    for (auto ptr: ptrs) {
        delete ptr;
    }
});

NONIUS_BENCHMARK("ptrs make_shared", [](nonius::chronometer meter)
{
    std::vector<std::shared_ptr<obj_type>> ptrs(meter.runs());
    meter.measure([&ptrs] (size_t i) {
        ptrs[i] = std::make_shared<obj_type>();
    });
});

NONIUS_BENCHMARK("ptrs gc_new", [](nonius::chronometer meter)
{
    gc_new<obj_type>();
    meter.measure([] {
        gc_ptr<obj_type> p = gc_new<obj_type>();
    });
});

//NONIUS_BENCHMARK("ptrs raw_ptr ctor", [](nonius::chronometer meter)
//{
//    nonius::storage_for<obj_type*> storage;
//    meter.measure([&storage] {
//        storage.construct();
//    });
//});

//NONIUS_BENCHMARK("ptrs shared_ptr default ctor", [](nonius::chronometer meter)
//{
//    nonius::storage_for<std::shared_ptr<obj_type>> storage;
//    meter.measure([&storage] {
//        storage.construct();
//    });
//});

NONIUS_BENCHMARK("ptrs shared_ptr ctor", [](nonius::chronometer meter)
{
    nonius::storage_for<std::shared_ptr<obj_type>> storage;
    obj_type* p = new obj_type(42);
    meter.measure([&storage, &p] {
        storage.construct(p);
    });
});

NONIUS_BENCHMARK("ptrs gc_ptr ctor", [](nonius::chronometer meter)
{
    nonius::storage_for<gc_ptr<obj_type>> storage;
    details::ptrs::gc_new_stack::activation_entry activation_entry;
    meter.measure([&storage] {
        storage.construct();
    });
});

NONIUS_BENCHMARK("ptrs gc_ptr root ctor", [](nonius::chronometer meter)
{
    nonius::storage_for<gc_ptr<obj_type>> storage;
    meter.measure([&storage] {
        storage.construct();
    });
});

//NONIUS_BENCHMARK("ptrs raw_ptr assign", [](nonius::chronometer meter)
//{
//    obj_type* a;
//    obj_type v = 42;
//    obj_type* b = &v;
//    meter.measure([&a, &b] {
//        a = b;
//    });
//});

NONIUS_BENCHMARK("ptrs shared_ptr assign", [](nonius::chronometer meter)
{
    std::shared_ptr<obj_type> a;
    auto b = std::make_shared<obj_type>(42);
    meter.measure([&a, &b] {
        a = b;
    });
});

NONIUS_BENCHMARK("ptrs gc_ptr assign", [](nonius::chronometer meter)
{
    gc_ptr<obj_type> a;
    auto b = gc_new<obj_type>(42);
    meter.measure([&a, &b] {
        a = b;
    });
});

//NONIUS_BENCHMARK("gc_ptr_assign_wb", [](nonius::chronometer meter)
//{
//    std::vector<gc_ptr<obj_type>> ptrs(meter.runs());
//    auto p = gc_new<obj_type>(42);
//    gc_garbage_collector::instance().force_move_to_marking();
//    meter.measure([&ptrs, &p] (size_t i) {
//        ptrs[i] = p;
//    });
//});

//NONIUS_BENCHMARK("ptrs raw_ptr dereference", [](nonius::chronometer meter)
//{
//    std::vector<obj_type*> ptrs(meter.runs());
//    for (auto& ptr: ptrs) {
//        ptr = new obj_type(42);
//    }
//    meter.measure([&ptrs] (size_t i) {
//        return *ptrs[i];
//    });
//    for (auto& ptr: ptrs) {
//        delete ptr;
//    }
//});

//NONIUS_BENCHMARK("ptrs shared_ptr dereference", [](nonius::chronometer meter)
//{
//    std::shared_ptr<obj_type> p = std::make_shared<obj_type>(42);
//    meter.measure([&p] {
//        return *p;
//    });
//});

NONIUS_BENCHMARK("ptrs gc_ptr dereference", [](nonius::chronometer meter)
{
    gc_ptr<obj_type> p = gc_new<obj_type>(42);
    meter.measure([&p] {
        gc_pin<obj_type> pin(p);
        return *pin;
    });
});