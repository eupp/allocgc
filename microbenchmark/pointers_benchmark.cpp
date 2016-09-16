#include <nonius/nonius.h++>

#include <vector>
#include <cstdlib>
#include <memory>

#include <libprecisegc/libprecisegc.hpp>
#include <libprecisegc/details/threads/gc_new_stack.hpp>

#include "deoptimize.hpp"

using namespace precisegc;

struct test_type
{
    size_t data[4];
};

typedef test_type obj_type;

NONIUS_BENCHMARK("pointers.new", [](nonius::chronometer meter)
{
    std::vector<obj_type*> ptrs(meter.runs());
    meter.measure([&ptrs] (size_t i) {
        ptrs[i] = new obj_type;
    });
    for (auto ptr: ptrs) {
        delete ptr;
    }
});

NONIUS_BENCHMARK("pointers.make_shared", [](nonius::chronometer meter)
{
    std::vector<std::shared_ptr<obj_type>> ptrs(meter.runs());
    meter.measure([&ptrs] (size_t i) {
        ptrs[i] = std::make_shared<obj_type>();
    });
});

NONIUS_BENCHMARK("pointers.gc_new", [](nonius::chronometer meter)
{
    gc_new<obj_type>();
    meter.measure([] {
        gc_ptr<obj_type> p = gc_new<obj_type>();
    });
});

NONIUS_BENCHMARK("pointers.raw_ptr.ctor", [](nonius::chronometer meter)
{
    nonius::storage_for<obj_type*> storage;
    meter.measure([&storage] {
        storage.construct();
        escape(&storage);
    });
});

NONIUS_BENCHMARK("pointers.shared_ptr.default_ctor", [](nonius::chronometer meter)
{
    nonius::storage_for<std::shared_ptr<obj_type>> storage;
    meter.measure([&storage] {
        storage.construct();
        escape(&storage);
    });
});

NONIUS_BENCHMARK("pointers.shared_ptr.ctor", [](nonius::chronometer meter)
{
    nonius::storage_for<std::shared_ptr<obj_type>> storage;
    obj_type* p = new obj_type();
    meter.measure([&storage, &p] {
        storage.construct(p);
    });
});

NONIUS_BENCHMARK("pointers.gc_ptr.ctor", [](nonius::chronometer meter)
{
    nonius::storage_for<gc_ptr<obj_type>> storage;
    details::threads::gc_new_stack::stack_entry stack_entry(
            reinterpret_cast<details::byte*>(&storage),
            sizeof(nonius::storage_for<gc_ptr<obj_type>>),
            false
    );
    meter.measure([&storage] {
        storage.construct();
    });
});

NONIUS_BENCHMARK("pointers.gc_ptr.root_ctor", [](nonius::chronometer meter)
{
    nonius::storage_for<gc_ptr<obj_type>> storage;
    meter.measure([&storage] {
        storage.construct();
    });
});

NONIUS_BENCHMARK("pointers.raw_ptr.assign", [](nonius::chronometer meter)
{
    obj_type* a;
    obj_type v;
    obj_type* b = &v;
    meter.measure([&a, &b] {
        a = b;
        clobber();
    });
});

NONIUS_BENCHMARK("pointers.shared_ptr.assign", [](nonius::chronometer meter)
{
    std::shared_ptr<obj_type> a;
    auto b = std::make_shared<obj_type>();
    meter.measure([&a, &b] {
        a = b;
    });
});

NONIUS_BENCHMARK("pointers.gc_ptr.assign", [](nonius::chronometer meter)
{
    gc_ptr<obj_type> a;
    auto b = gc_new<obj_type>();
    meter.measure([&a, &b] {
        a = b;
    });
});

NONIUS_BENCHMARK("pointers.gc_ptr.assign_wb", [](nonius::chronometer meter)
{
    using namespace precisegc::details;

    gc_ptr<obj_type> a;
    auto b = gc_new<obj_type>();
    gc_initiation_point(initiation_point_type::START_MARKING, initiation_point_data::create_empty_data());
    meter.measure([&a, &b] (size_t i) {
        a = b;
    });
    gc_initiation_point(initiation_point_type::START_COLLECTING, initiation_point_data::create_empty_data());
});

NONIUS_BENCHMARK("pointers.raw_ptr.dereference", [](nonius::chronometer meter)
{
    volatile obj_type* p = new obj_type();
    meter.measure([&p] () -> volatile obj_type& {
        volatile obj_type& v = *p;
        escape(const_cast<obj_type*>(&v));
        return v;
    });
    delete p;
});

NONIUS_BENCHMARK("pointers.shared_ptr.dereference", [](nonius::chronometer meter)
{
    std::shared_ptr<obj_type> p = std::make_shared<obj_type>();
    meter.measure([&p] () -> volatile obj_type& {
        volatile obj_type& v = *p;
        escape(const_cast<obj_type*>(&v));
        return v;
    });
});

NONIUS_BENCHMARK("pointers.gc_ptr.dereference", [](nonius::chronometer meter)
{
    gc_ptr<obj_type> p = gc_new<obj_type>();
    meter.measure([&p] {
        return *p;
    });
});