#include <gtest/gtest.h>

#include <libprecisegc/details/allocators/types.h>
#include <libprecisegc/details/utils/flattened_range.hpp>

#include <memory>
#include <iostream>

#include <boost/range/iterator_range.hpp>
#include <boost/range/adaptor/reversed.hpp>

#include "rand_util.h"

using namespace precisegc::details;
using namespace precisegc::details::utils;

namespace {
static const size_t RANGE_SIZE = 4;
}

struct test_range
{
public:
    typedef byte* iterator;

    test_range() = default;

    template <typename Generator>
    test_range(Generator gen)
        : mem(new byte[RANGE_SIZE])
    {
        std::generate(mem.get(), mem.get() + RANGE_SIZE, gen);
    }

    iterator begin() const
    {
        return mem.get();
    }

    iterator end() const
    {
        return mem ? mem.get() + RANGE_SIZE : nullptr;
    }

    std::unique_ptr<byte[]> mem;
};

TEST(flatten_range_test, test_empty)
{
    std::vector<test_range> vec;
    auto rng = flatten_range(vec);
    ASSERT_EQ(rng.begin(), rng.end());
}

TEST(flatten_range_test, test_pass_through_1)
{
    static const size_t VEC_SIZE = 3;

    uniform_rand_generator<byte> gen(0, std::numeric_limits<byte>::max());
    std::vector<test_range> vec;
    vec.emplace_back(gen);
    vec.emplace_back(gen);
    vec.emplace_back(gen);

    std::vector<byte> expected;
    for (size_t i = 0; i < VEC_SIZE; ++i) {
        auto& rng = vec[i];
        std::copy(rng.begin(), rng.end(), std::back_inserter(expected));
    }

    auto flattened = flatten_range(vec);
    ASSERT_TRUE(std::equal(flattened.begin(), flattened.end(), expected.begin()));

    auto reversed = flattened | boost::adaptors::reversed;

    ASSERT_TRUE(std::equal(reversed.begin(), reversed.end(), expected.rbegin()));
}

TEST(flatten_range_test, test_pass_through_2)
{
    static const size_t VEC_SIZE = 3;

    uniform_rand_generator<byte> gen(0, std::numeric_limits<byte>::max());
    std::vector<test_range> vec;
    vec.emplace_back(gen);
    vec.emplace_back();
    vec.emplace_back(gen);

    std::vector<byte> expected;
    for (size_t i = 0; i < VEC_SIZE; ++i) {
        auto& rng = vec[i];
        std::copy(rng.begin(), rng.end(), std::back_inserter(expected));
    }

    auto flattened = flatten_range(vec);
    ASSERT_TRUE(std::equal(flattened.begin(), flattened.end(), expected.begin()));

    auto reversed = flattened | boost::adaptors::reversed;

    ASSERT_TRUE(std::equal(reversed.begin(), reversed.end(), expected.rbegin()));
}

TEST(flatten_range_test, test_pass_through_3)
{
    static const size_t VEC_SIZE = 3;

    uniform_rand_generator<byte> gen(0, std::numeric_limits<byte>::max());
    std::vector<test_range> vec;
    vec.emplace_back();
    vec.emplace_back(gen);
    vec.emplace_back(gen);

    std::vector<byte> expected;
    for (size_t i = 0; i < VEC_SIZE; ++i) {
        auto& rng = vec[i];
        std::copy(rng.begin(), rng.end(), std::back_inserter(expected));
    }

    auto flattened = flatten_range(vec);
    ASSERT_TRUE(std::equal(flattened.begin(), flattened.end(), expected.begin()));

    auto reversed = flattened | boost::adaptors::reversed;

    ASSERT_TRUE(std::equal(reversed.begin(), reversed.end(), expected.rbegin()));
}

