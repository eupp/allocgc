#include <gtest/gtest.h>

#include <libprecisegc/details/threads/pin_stack.hpp>

#include <set>

using namespace precisegc::details;
using namespace precisegc::details::threads;

TEST(pin_stack_test, test_push)
{
    byte* ptr;
    pin_stack ps;
    ps.push_pin(ptr);

    ASSERT_TRUE(ps.contains(ptr));
    ASSERT_EQ(1, ps.count());
}

TEST(pin_stack_test, test_pop)
{
    byte* ptr;
    pin_stack ps;
    ps.push_pin(ptr);
    ps.pop_pin();

    ASSERT_FALSE(ps.contains(ptr));
    ASSERT_EQ(0, ps.count());
}

TEST(pin_stack_test, test_trace)
{
    const size_t PIN_COUNT = 3;
    byte* ptrs[PIN_COUNT];
    std::set<byte*> expected(ptrs, ptrs + PIN_COUNT);
    std::set<byte*> actual;

    pin_stack ps;
    for (auto& ptr: ptrs) {
        ps.push_pin(ptr);
    }

    ps.trace([&actual] (byte* pin) { actual.insert(pin); });

    ASSERT_EQ(expected, actual);
}
