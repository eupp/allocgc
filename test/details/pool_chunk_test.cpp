#include <gtest/gtest.h>

#include <memory>

#include "libprecisegc/details/pool_chunk.h"

using namespace precisegc::details;

class pool_chunk_test: public ::testing::Test
{
public:

    static const int CHUNK_SIZE = pool_chunk::CHUNK_MAXSIZE;
    static const int OBJ_SIZE = sizeof(size_t);

    pool_chunk_test()
            : mem(new size_t[CHUNK_SIZE])
            , chk((void*) mem.get(), OBJ_SIZE, CHUNK_SIZE)
    {}

    std::unique_ptr<size_t[]> mem;
    pool_chunk chk;
};

TEST_F(pool_chunk_test, test_is_memory_available)
{
    ASSERT_TRUE(chk.is_memory_available());

    for (int i = 0; i < CHUNK_SIZE; ++i) {
        ASSERT_TRUE(chk.is_memory_available());
        size_t* ptr = (size_t*) chk.allocate(OBJ_SIZE);
        *ptr = 42;
    }

    ASSERT_FALSE(chk.is_memory_available());
}

TEST_F(pool_chunk_test, test_allocate)
{
    size_t* ptr1 = (size_t*) chk.allocate(OBJ_SIZE);
    EXPECT_NE(nullptr, ptr1);
    // try dereference pointer
    // in case of incorrect allocation it might cause segmentation fault
    *ptr1 = 42;

    size_t* ptr2 = (size_t*) chk.allocate(OBJ_SIZE);
    EXPECT_NE(nullptr, ptr2);
    EXPECT_NE(ptr1, ptr2);
    *ptr2 = 42;
}

TEST_F(pool_chunk_test, test_deallocate)
{
    size_t* ptr = nullptr;
    for (int i = 0; i < CHUNK_SIZE; ++i) {
        ptr = (size_t*) chk.allocate(OBJ_SIZE);
    }

    chk.deallocate((void*) ptr, OBJ_SIZE);
    ASSERT_TRUE(chk.is_memory_available());

    ptr = (size_t*) chk.allocate(OBJ_SIZE);
    EXPECT_NE(nullptr, ptr);
    EXPECT_FALSE(chk.is_memory_available());
}