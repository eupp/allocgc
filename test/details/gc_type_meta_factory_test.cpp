#include <gtest/gtest.h>

#include <thread>
#include <vector>
#include <algorithm>

#include <libprecisegc/details/gc_type_meta.hpp>
#include <libprecisegc/details/gc_type_meta_factory.hpp>
#include <libprecisegc/details/utils/barrier.hpp>
#include <libprecisegc/details/utils/scoped_thread.hpp>

using namespace std;
using namespace precisegc::details;

struct type_1 {};
struct type_2 {};
struct type_3 {};

TEST(gc_type_meta_factory_test, test_get)
{
    EXPECT_EQ(nullptr, gc_type_meta_factory<type_1>::get());
}

TEST(gc_type_meta_factory_test, test_create)
{
    typedef gc_type_meta_factory<type_2> factory;
    vector<size_t> offsets({1, 2, 3});
    factory::create(offsets.begin(), offsets.end());
    EXPECT_NE(nullptr, factory::get());
    EXPECT_EQ(sizeof(type_2), factory::get()->type_size());
    EXPECT_EQ(offsets.size(), factory::get()->offsets().size());
    EXPECT_TRUE(std::equal(offsets.begin(), offsets.end(), factory::get()->offsets().begin()));
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

    EXPECT_NE(nullptr, factory::get());
    EXPECT_EQ(sizeof(type_3), factory::get()->type_size());
    EXPECT_EQ(offsets.size(), factory::get()->offsets().size());
    EXPECT_TRUE(std::equal(offsets.begin(), offsets.end(), factory::get()->offsets().begin()));
}

