#include <nonius/nonius.h++>

#include <vector>
#include <cstdlib>
#include <memory>

#include "libprecisegc/libprecisegc.h"
#include "libprecisegc/details/allocators/index_tree.h"
#include "libprecisegc/details/gc_new_stack.h"

using namespace precisegc;
using namespace precisegc::details;
using namespace precisegc::details::allocators;

typedef size_t obj_type;

//NONIUS_BENCHMARK("new", [](nonius::chronometer meter)
//{
//    std::vector<obj_type*> ptrs(meter.runs());
//    meter.measure([&ptrs] (size_t i) {
//        obj_type* obj = new obj_type();
//        ptrs[i] = obj;
//        return obj;
//    });
//    for (auto ptr: ptrs) {
//        delete ptr;
//    }
//});
//
//NONIUS_BENCHMARK("make_shared", [](nonius::chronometer meter)
//{
//    std::vector<std::shared_ptr<obj_type>> ptrs(meter.runs());
//    meter.measure([&ptrs] (size_t i) {
//        auto obj = std::make_shared<obj_type>();
//        ptrs[i] = obj;
//        return obj;
//    });
//});
//
//NONIUS_BENCHMARK("gc_new", [](nonius::chronometer meter)
//{
//    gc_init();
//    gc_new<obj_type>();
//    meter.measure([] {
//        gc_ptr<obj_type> p = gc_new<obj_type>();
//        return p;
//    });
//});
//
//NONIUS_BENCHMARK("raw_ptr_constructor", [](nonius::chronometer meter)
//{
//    std::vector<nonius::storage_for<obj_type*>> storage(meter.runs());
//    meter.measure([&storage] (size_t i) {
//        storage[i].construct();
//    });
//});
//
//NONIUS_BENCHMARK("shared_ptr_null_constructor", [](nonius::chronometer meter)
//{
//    std::vector<nonius::storage_for<std::shared_ptr<obj_type>>> storage(meter.runs());
//    meter.measure([&storage] (size_t i) {
//        storage[i].construct();
//    });
//});
//
//NONIUS_BENCHMARK("shared_ptr_constructor", [](nonius::chronometer meter)
//{
//    std::vector<nonius::storage_for<std::shared_ptr<obj_type>>> storage(meter.runs());
//    std::vector<obj_type*> ptrs(meter.runs());
//    for (auto& ptr: ptrs) {
//        ptr = new obj_type(42);
//    }
//    meter.measure([&storage, &ptrs] (size_t i) {
//        storage[i].construct(ptrs[i]);
//    });
//});
//
//NONIUS_BENCHMARK("gc_ptr_constructor", [](nonius::chronometer meter)
//{
//    gc_init();
//    std::vector<nonius::storage_for<gc_ptr<obj_type>>> storage(meter.runs());
//    gc_new_stack::activation_entry activation_entry;
//    meter.measure([&storage] (size_t i) {
//        storage[i].construct();
//    });
//});
//
//NONIUS_BENCHMARK("gc_ptr_root_constructor", [](nonius::chronometer meter)
//{
//    gc_init();
//    std::vector<nonius::storage_for<gc_ptr<obj_type>>> storage(meter.runs());
//    meter.measure([&storage] (size_t i) {
//        storage[i].construct();
//    });
//});
//
//NONIUS_BENCHMARK("raw_ptr_assign", [](nonius::chronometer meter)
//{
//    std::vector<obj_type*> ptrs(meter.runs());
//    obj_type v = 42;
//    obj_type* p = &v;
//    meter.measure([&ptrs, p] (size_t i) {
//        ptrs[i] = p;
//    });
//});
//
//NONIUS_BENCHMARK("shared_ptr_assign", [](nonius::chronometer meter)
//{
//    std::vector<std::shared_ptr<obj_type>> ptrs(meter.runs());
//    auto p = std::make_shared<obj_type>(42);
//    meter.measure([&ptrs, p] (size_t i) {
//        ptrs[i] = p;
//    });
//});
//
//NONIUS_BENCHMARK("gc_ptr_assign", [](nonius::chronometer meter)
//{
//    std::vector<gc_ptr<obj_type>> ptrs(meter.runs());
//    auto p = gc_new<obj_type>(42);
//    meter.measure([&ptrs, &p] (size_t i) {
//        ptrs[i] = p;
//    });
//});
//
//NONIUS_BENCHMARK("gc_ptr_assign_wb", [](nonius::chronometer meter)
//{
//    std::vector<gc_ptr<obj_type>> ptrs(meter.runs());
//    auto p = gc_new<obj_type>(42);
//    gc_garbage_collector::instance().force_move_to_marking();
//    meter.measure([&ptrs, &p] (size_t i) {
//        ptrs[i] = p;
//    });
//});
//
//NONIUS_BENCHMARK("raw_ptr_deref", [](nonius::chronometer meter)
//{
//    std::vector<obj_type*> ptrs(meter.runs());
//    for (auto& ptr: ptrs) {
//        ptr = new obj_type(42);
//    }
//    meter.measure([&ptrs] (size_t i) {
//        obj_type o = *ptrs[i];
//        return o;
//    });
//    for (auto& ptr: ptrs) {
//        delete ptr;
//    }
//});
//
//NONIUS_BENCHMARK("shared_ptr_deref", [](nonius::chronometer meter)
//{
//    std::vector<std::shared_ptr<obj_type>> ptrs(meter.runs());
//    for (auto& ptr: ptrs) {
//        ptr = std::make_shared<obj_type>(42);
//    }
//    meter.measure([&ptrs] (size_t i) {
//        obj_type o = *ptrs[i];
//        return o;
//    });
//});
//
//NONIUS_BENCHMARK("gc_ptr_deref", [](nonius::chronometer meter)
//{
//    std::vector<gc_ptr<obj_type>> ptrs(meter.runs());
//    for (auto& ptr: ptrs) {
//        ptr = gc_new<obj_type>(42);
//    }
//    meter.measure([&ptrs] (size_t i) {
//        gc_pin<obj_type> pin(ptrs[i]);
//        obj_type o = *pin;
//        return o;
//    });
//});