#include <gtest/gtest.h>

#include <vector>
#include <iterator>
#include <algorithm>
#include <limits>

#include "libprecisegc/details/allocators/types.h"
#include "libprecisegc/details/allocators/joined_range.h"
#include "libprecisegc/details/allocators/iterator_range.h"
#include "libprecisegc/details/allocators/reversed_range.h"

#include "rand_util.h"

using namespace precisegc::details;
using namespace precisegc::details::allocators;

namespace {
static const size_t RANGE_SIZE = 64;

struct test_range
{
public:
    typedef byte* iterator;
    typedef iterator_range<byte*> range_type;

    test_range()
    {
        uniform_rand_generator<byte> gen(0, std::numeric_limits<byte>::max());
        std::generate(mem, mem + RANGE_SIZE, gen);
    }

    range_type get_range()
    {
        return range_type(mem, mem + RANGE_SIZE);
    }

    byte mem[RANGE_SIZE];
};
}

TEST(join_range_test, test_empty)
{
    std::vector<test_range> vec;
    joined_range<decltype(vec)> rng(vec);
    ASSERT_EQ(rng.begin(), rng.end());
}

TEST(join_range_test, test_forward_pass)
{
    static const size_t VEC_SIZE = 3;

    std::vector<test_range> vec;
    std::vector<byte> joined;
    for (size_t i = 0; i < VEC_SIZE; ++i) {
        vec.emplace_back();
        byte* mem = vec.back().mem;
        std::copy(mem, mem + RANGE_SIZE, std::back_inserter(joined));
    }

    joined_range<decltype(vec)> rng(vec);
    ASSERT_TRUE(std::equal(rng.begin(), rng.end(), joined.begin()));
}

#include <iostream>

TEST(join_range_test, test_reverse_pass)
{
    static const size_t VEC_SIZE = 3;

    std::vector<test_range> vec;
    std::vector<byte> joined;
    for (size_t i = 0; i < VEC_SIZE; ++i) {
        vec.emplace_back();
        byte* mem = vec.back().mem;
        std::copy(mem, mem + RANGE_SIZE, std::back_inserter(joined));
    }

    joined_range<decltype(vec)> rng(vec);
    reversed_range<decltype(rng)> rev_rng(rng);

    auto it1 = rev_rng.begin();
    auto it2 = joined.rbegin();

//    for (; )

    ASSERT_EQ(joined.size(), std::distance(rev_rng.begin(), rev_rng.end()));
    ASSERT_TRUE(std::equal(rev_rng.begin(), rev_rng.end(), joined.rbegin()));
}