#include <gtest/gtest.h>

#include <atomic>

#include <liballocgc/details/utils/bitset.hpp>
#include <liballocgc/details/utils/scoped_thread.hpp>

using namespace allocgc::details::utils;

namespace {
static const size_t ULL_SIZE = CHAR_BIT * sizeof(unsigned long long);
}

TEST(bitset_test, test_get_set)
{
    static const size_t SIZE = 4 * ULL_SIZE;
    bitset<SIZE> bits;

    ASSERT_FALSE(bits.all());
    ASSERT_FALSE(bits.any());
    ASSERT_TRUE(bits.none());

    for (size_t i = 0; i < SIZE; ++i) {
        ASSERT_FALSE(bits.get(i));
        bits.set(i, true);
        ASSERT_TRUE(bits.get(i));

        if (i < SIZE - 1) {
            ASSERT_FALSE(bits.all());
        }
        ASSERT_TRUE(bits.any());
        ASSERT_FALSE(bits.none());
    }

    ASSERT_TRUE(bits.all());
}

TEST(bitset_test, test_set_reset_all)
{
    static const size_t SIZE = 4 * ULL_SIZE;
    bitset<SIZE> bits;

    bits.set_all();
    ASSERT_TRUE(bits.all());

    bits.reset_all();
    ASSERT_TRUE(bits.none());
}

TEST(bitset_test, test_left_shift)
{
    static const size_t SIZE = 4 * ULL_SIZE;
    bitset<SIZE> bits;

    int small_shift = ULL_SIZE / 2;
    bits.set(0, true);
    bits <<= small_shift;
    ASSERT_TRUE(bits.get(small_shift));

    bits.set(small_shift, false);
    ASSERT_TRUE(bits.none());

    int large_shift = ULL_SIZE + 1;
    bits.set(0, true);
    bits.set(1, true);
    bits <<= large_shift;
    ASSERT_TRUE(bits.get(large_shift));
    ASSERT_TRUE(bits.get(large_shift + 1));


    bits.set(large_shift, false);
    bits.set(large_shift + 1, false);
    ASSERT_TRUE(bits.none());
}

TEST(bitset_test, test_right_shift)
{
    static const size_t SIZE = 4 * ULL_SIZE;
    bitset<SIZE> bits;

    int small_shift = ULL_SIZE / 2;
    bits.set(small_shift, true);
    bits >>= small_shift;
    ASSERT_TRUE(bits.get(0));

    bits.set(0, false);
    ASSERT_TRUE(bits.none());

    int large_shift = ULL_SIZE + 1;
    bits.set(large_shift, true);
    bits.set(large_shift + 1, true);
    bits >>= large_shift;
    ASSERT_TRUE(bits.get(0));
    ASSERT_TRUE(bits.get(1));


    bits.set(0, false);
    bits.set(1, false);
    ASSERT_TRUE(bits.none());
}

TEST(bitset_test, test_msb)
{
    static const size_t SIZE = 4 * ULL_SIZE;
    bitset<SIZE> bits;

    size_t bit_idx = ULL_SIZE / 2 + 1;

    bits.set(bit_idx);
    ASSERT_EQ(bit_idx, *bits.most_significant_bit());

    bits.reset(bit_idx);
    bit_idx = 0;
    bits.set(bit_idx);
    ASSERT_EQ(bit_idx, *bits.most_significant_bit());

    bits.reset(bit_idx);
    ASSERT_FALSE(bits.most_significant_bit().is_initialized());
}


TEST(sync_bitset_test, test_consistency)
{
    static const size_t SIZE = 2 * ULL_SIZE;
    sync_bitset<SIZE> bits;

    scoped_thread threads[SIZE];
    for (size_t i = 0; i < SIZE; ++i) {
        threads[i] = std::thread([i, &bits] {
            bits.set(i, true);
        });
    }

    for (size_t i = 0; i < SIZE; ++i) {
        threads[i].join();
    }

    ASSERT_EQ(SIZE, bits.count());
    ASSERT_TRUE(bits.all());
}

TEST(sync_bitset_test, test_synchronization)
{
    static const size_t SIZE = ULL_SIZE;
    sync_bitset<SIZE> bits;

    size_t i = 0;
    size_t j = 1;

    scoped_thread write_i_then_j = std::thread([i, j, &bits] {
        bits.set(i, true);
        bits.set(j, true);
    });

    scoped_thread read_j_then_i = std::thread([i, j, &bits] {
        while (!bits.get(j));
        ASSERT_TRUE(bits.get(i));
    });
}
