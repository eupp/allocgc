#include <gtest/gtest.h>

#include <thread>
#include <vector>
#include <algorithm>

#include <libprecisegc/details/type_meta.hpp>
#include <libprecisegc/details/utils/barrier.hpp>
#include <libprecisegc/details/utils/scoped_thread.hpp>

using namespace std;
using namespace precisegc::details;

struct type_1 {};
struct type_2 {};
struct type_3 {};

TEST(type_meta_test, test_is_initialized)
{
    EXPECT_FALSE(type_meta_provider<type_1>::is_meta_created());
}

TEST(type_meta_test, test_create_meta)
{
    typedef type_meta_provider<type_2> provider;
    vector<size_t> offsets({1, 2, 3});
    provider::create_meta(offsets.begin(), offsets.end());
    EXPECT_TRUE(provider::is_meta_created());
    EXPECT_EQ(sizeof(type_2), provider::get_meta()->type_size());
    EXPECT_EQ(offsets.size(), provider::get_meta()->offsets().size());
    EXPECT_TRUE(std::equal(offsets.begin(), offsets.end(), provider::get_meta()->offsets().begin()));
}

TEST(type_meta_test, test_threading)
{
    typedef type_meta_provider<type_3> provider;

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
            provider::create_meta(offsets.begin(), offsets.end());
        });
    }

    for (auto& thread: threads) {
        thread.join();
    }

    EXPECT_TRUE(provider::is_meta_created());
    EXPECT_EQ(sizeof(type_3), provider::get_meta()->type_size());
    EXPECT_EQ(offsets.size(), provider::get_meta()->offsets().size());
    EXPECT_TRUE(std::equal(offsets.begin(), offsets.end(), provider::get_meta()->offsets().begin()));
}

