#include <gtest/gtest.h>

#include <memory>

#include "libprecisegc/details/segregated_list.h"

using namespace std;
using namespace precisegc::details;

static const size_t OBJ_SIZE = 8;

TEST(test_segregated_list_element, test_allocate)
{
    unique_ptr<segregated_list_element> sle(new segregated_list_element(OBJ_SIZE));
    for (int i = 0; i < PAGES_PER_SEGREGATED_STORAGE_ELEMENT; ++i) {
        ASSERT_TRUE(sle->is_memory_available());
        auto alloc_res = sle->allocate();
        page_descriptor* pd = alloc_res.second;
        for (int j = 1; j < pd->page_size() / OBJ_SIZE; ++j) {
            ASSERT_TRUE(sle->is_memory_available());
            auto alloc_res = sle->allocate();
            size_t* ptr = (size_t*) alloc_res.first;
            EXPECT_NE(nullptr, ptr);
            EXPECT_EQ(pd, alloc_res.second);
            *ptr = 42;
        }
    }
    ASSERT_FALSE(sle->is_memory_available());
}

TEST(test_segregated_list, test_allocate)
{
    segregated_list sl(OBJ_SIZE);
    for (int i = 0 ; i < PAGES_PER_SEGREGATED_STORAGE_ELEMENT; ++i) {
        auto alloc_res = sl.allocate();
        page_descriptor* pd = alloc_res.second;
        for (int j = 0; j < pd->page_size(); ++j) {
            sl.allocate();
        }
    }
    auto alloc_res = sl.allocate();
    size_t* ptr = (size_t*) alloc_res.first;
    ASSERT_NE(nullptr, ptr);
    ASSERT_NE(nullptr, alloc_res.second);
    *ptr = 42;
}