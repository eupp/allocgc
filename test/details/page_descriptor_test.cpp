#include <gtest/gtest.h>

#include "libprecisegc/details/page_descriptor.h"

using namespace precisegc::details;

static const size_t PAGE_SIZE = 64;
static const size_t OBJ_SIZE = 8;

TEST(page_descriptor_test, test_constructor)
{
    page_descriptor pd;
    EXPECT_FALSE(pd.is_memory_available(1));
}

TEST(page_descriptor_test, test_initialize_page)
{
    page_descriptor pd;
    pd.initialize_page(PAGE_SIZE, OBJ_SIZE);
    EXPECT_TRUE(pd.is_memory_available(OBJ_SIZE));
    EXPECT_TRUE(pd.is_memory_available(PAGE_SIZE));
}

TEST(page_descriptor_test, test_allocate)
{
    page_descriptor pd;
    pd.initialize_page(PAGE_SIZE, OBJ_SIZE);
    for (int i = 0; i < PAGE_SIZE / OBJ_SIZE; ++i) {
        size_t* ptr = (size_t*) pd.allocate(OBJ_SIZE);
        ASSERT_NE(nullptr, ptr);
        *ptr = 42;
    }
    EXPECT_FALSE(pd.is_memory_available(OBJ_SIZE));
    EXPECT_FALSE(pd.is_memory_available(1));
}

TEST(page_descriptor_test, test_get_object_start)
{
    page_descriptor pd;
    pd.initialize_page(PAGE_SIZE, OBJ_SIZE);
    for (int i = 0; i < PAGE_SIZE / OBJ_SIZE; ++i) {
        void* ptr = pd.allocate(OBJ_SIZE);
        for (int j = 0; j < OBJ_SIZE; ++j) {
            void* ptr_ = (void*) ((size_t) ptr + j);
            ASSERT_EQ(ptr, pd.get_object_start(ptr_));
        }
    }
}

