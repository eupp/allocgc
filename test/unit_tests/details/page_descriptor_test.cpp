#include <gtest/gtest.h>

#include "libprecisegc/details/page_descriptor.h"

using namespace precisegc::details;

static const size_t OBJ_SIZE = MEMORY_CELL_SIZE / OBJECTS_PER_PAGE;
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
    EXPECT_EQ(OBJ_SIZE, pd.obj_size());
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

TEST(page_descriptor_test, test_uninitialized_iterators)
{
    page_descriptor pd;
    ASSERT_EQ(pd.begin(), pd.end());
}

TEST(page_descriptor_test, test_initialized_iterators)
{
    page_descriptor pd;
    pd.initialize_page(OBJ_SIZE);
    int total_cnt = pd.page_size() / OBJ_SIZE;

    pd.allocate();
    int cnt = 0;
    for (auto it = pd.begin(); it != pd.end(); ++it) {
        cnt++;
        size_t* ptr = (size_t*) *it;
        ASSERT_NE(nullptr, ptr);
        *ptr = 42;
    }
    ASSERT_EQ(1, cnt);

    for (int i = 0; i < total_cnt - 1; ++i) {
        pd.allocate();
    }
    cnt = 0;
    for (auto it = pd.begin(); it != pd.end(); ++it) {
        ++cnt;
        size_t* ptr = (size_t*) *it;
        ASSERT_NE(nullptr, ptr);
        *ptr = 42;
    }
    ASSERT_EQ(total_cnt, cnt);
}

TEST(page_descriptor_test, test_initialized_iterators_reverse)
{
    page_descriptor pd;
    pd.initialize_page(OBJ_SIZE);
    int total_cnt = pd.page_size() / OBJ_SIZE;
    for (int i = 0; i < total_cnt; ++i) {
        pd.allocate();
    }

    int cnt = 0;
    for (auto it = pd.end(); it != pd.begin(); --it) {
        ++cnt;
        if (it != pd.end()) {
            size_t* ptr = (size_t*) *it;
            ASSERT_NE(nullptr, ptr);
            *ptr = 42;
        }
    }
    ASSERT_EQ(total_cnt, cnt);
}

TEST(page_descriptor_test, test_iterator_set_deallocated)
{
    page_descriptor pd;
    pd.initialize_page(OBJ_SIZE);
    int total_cnt = pd.page_size() / OBJ_SIZE;

    pd.allocate();
    ASSERT_NE(pd.begin(), pd.end());

    auto it = pd.begin();
    void* ptr = *it;
    it.set_deallocated();
    ASSERT_EQ(pd.begin(), pd.end());

    pd.allocate();
    it = pd.begin();
    ASSERT_EQ(ptr, *it);
}

