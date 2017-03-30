#include <gtest/gtest.h>

#include <thread>
#include <vector>
#include <algorithm>

#include <libprecisegc/gc_type_meta.hpp>
#include <libprecisegc/details/utils/barrier.hpp>
#include <libprecisegc/details/utils/scoped_thread.hpp>

using namespace std;

using namespace precisegc;
using namespace precisegc::details;

namespace {
struct type_1 {};
struct type_2 {};
struct type_3 {};

struct type_with_dtor
{
    ~type_with_dtor()
    {
        dtor_called = true;
    }

    static bool dtor_called;
};

bool type_with_dtor::dtor_called = false;

struct type_with_move_ctor
{
    type_with_move_ctor() = default;

    type_with_move_ctor(type_with_move_ctor&&)
    {
        move_ctor_called = true;
        to_ptr = this;
    }

    static bool move_ctor_called;
    static type_with_move_ctor* to_ptr;
};

bool type_with_move_ctor::move_ctor_called = false;
type_with_move_ctor* type_with_move_ctor::to_ptr = nullptr;

struct nonmovable_type : private utils::noncopyable, private utils::nonmovable
{
    nonmovable_type() = default;
};

}

TEST(gc_type_meta_factory_test, test_get)
{
    EXPECT_EQ(nullptr, gc_type_meta_factory<type_1>::get());
}

TEST(gc_type_meta_factory_test, test_create)
{
    typedef gc_type_meta_factory<type_2> factory;
    vector<size_t> offsets({1, 2, 3});
    factory::create(offsets.begin(), offsets.end());
    ASSERT_NE(nullptr, factory::get());
    EXPECT_EQ(sizeof(type_2), factory::get()->type_size());
    EXPECT_EQ(offsets.size(), factory::get()->offsets().size());
    EXPECT_TRUE(std::equal(offsets.begin(), offsets.end(), factory::get()->offsets().begin()));
}

TEST(gc_type_meta_factory_test, test_destroy)
{
    typedef gc_type_meta_factory<type_with_dtor> factory;
    factory::create();

    const gc_type_meta* tmeta = factory::get();
    ASSERT_NE(nullptr, tmeta);
    ASSERT_FALSE(type_with_dtor::dtor_called);

    std::aligned_storage<sizeof(type_with_dtor)> storage;
    byte* ptr = reinterpret_cast<byte*>(&storage);
    new (ptr) type_with_dtor();

    tmeta->destroy(ptr);

    ASSERT_TRUE(type_with_dtor::dtor_called);
}

TEST(gc_type_meta_factory_test, test_move)
{
    typedef gc_type_meta_factory<type_with_move_ctor> factory;
    factory::create();

    const gc_type_meta* tmeta = factory::get();
    ASSERT_NE(nullptr, tmeta);
    ASSERT_FALSE(type_with_move_ctor::move_ctor_called);

    std::aligned_storage<sizeof(type_with_move_ctor)> from_storage;
    byte* from = reinterpret_cast<byte*>(&from_storage);
    new (from) type_with_move_ctor();

    std::aligned_storage<sizeof(type_with_move_ctor)> to_storage;
    byte* to = reinterpret_cast<byte*>(&to_storage);

    tmeta->move(from, to);

    ASSERT_TRUE(type_with_move_ctor::move_ctor_called);
    ASSERT_EQ(to, (byte*) type_with_move_ctor::to_ptr);
}

TEST(gc_type_meta_factory_test, test_forbidden_move)
{
    typedef gc_type_meta_factory<nonmovable_type> factory;
    factory::create();

    const gc_type_meta* tmeta = factory::get();
    ASSERT_NE(nullptr, tmeta);

    std::aligned_storage<sizeof(nonmovable_type)> from_storage;
    byte* from = reinterpret_cast<byte*>(&from_storage);
    new (from) nonmovable_type();

    std::aligned_storage<sizeof(nonmovable_type)> to_storage;
    byte* to = reinterpret_cast<byte*>(&to_storage);

    ASSERT_FALSE(tmeta->is_movable());
    ASSERT_THROW(tmeta->move(from, to), forbidden_move_exception);
}

TEST(gc_type_meta_factory_test, test_multithreading)
{
    typedef gc_type_meta_factory<type_3> factory;

    const int THREADS_COUNT = 100;
    utils::scoped_thread threads[THREADS_COUNT];

    utils::barrier barrier(THREADS_COUNT);
    vector<size_t> offsets;
    for (int i = 0; i < 1000; ++i) {
        offsets.push_back(i);
    }

    for (auto& thread: threads) {
        thread = std::thread([&offsets, &barrier] {
            barrier.wait();
            factory::create(offsets.begin(), offsets.end());
        });
    }

    for (auto& thread: threads) {
        thread.join();
    }

    ASSERT_NE(nullptr, factory::get());
    EXPECT_EQ(sizeof(type_3), factory::get()->type_size());
    EXPECT_EQ(offsets.size(), factory::get()->offsets().size());
    EXPECT_TRUE(std::equal(offsets.begin(), offsets.end(), factory::get()->offsets().begin()));
}

