#include <gtest/gtest.h>

#include "libprecisegc/details/page_descriptor.h"

using namespace precisegc::details;

static const size_t OBJ_SIZE = 8;
static const size_t PAGE_SIZE = OBJ_SIZE << OBJECTS_PER_PAGE_BITS;

TEST(page_descriptor_test, test_constructor)
{
    page_descriptor pd;
    EXPECT_FALSE(pd.is_memory_available());
}

TEST(page_descriptor_test, test_initialize_page)
{
    page_descriptor pd;
    pd.initialize_page(OBJ_SIZE);
    EXPECT_TRUE(pd.is_memory_available());
}

TEST(page_descriptor_test, test_page_size)
{
    page_descriptor pd;
    pd.initialize_page(OBJ_SIZE);
    EXPECT_GE(pd.page_size(), OBJ_SIZE * OBJECTS_PER_PAGE);
}

TEST(page_descriptor_test, test_allocate)
{
    page_descriptor pd;
    pd.initialize_page(OBJ_SIZE);
    for (int i = 0; i < pd.page_size() / OBJ_SIZE; ++i) {
        ASSERT_TRUE(pd.is_memory_available());
        size_t* ptr = (size_t*) pd.allocate();
        ASSERT_NE(nullptr, ptr);
        *ptr = 42;
    }
    EXPECT_FALSE(pd.is_memory_available());
}

TEST(page_descriptor_test, test_get_object_start)
{
    page_descriptor pd;
    pd.initialize_page(OBJ_SIZE);
    for (int i = 0; i < pd.page_size() / OBJ_SIZE; ++i) {
        void* ptr = pd.allocate();
        for (int j = 0; j < OBJ_SIZE; ++j) {
            void* ptr_ = (void*) ((size_t) ptr + j);
            ASSERT_EQ(ptr, pd.get_object_start(ptr_));
        }
    }
}
