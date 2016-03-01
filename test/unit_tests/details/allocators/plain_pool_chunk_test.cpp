#include <gtest/gtest.h>

#include <memory>

#include "libprecisegc/details/allocators/plain_pool_chunk.h"

using namespace precisegc::details::allocators;

class plain_pool_chunk_test: public ::testing::Test
{
public:

    static const int CHUNK_SIZE = plain_pool_chunk::CHUNK_MAXSIZE;
    static const int OBJ_SIZE = sizeof(size_t);

    plain_pool_chunk_test()
            : m_mem(new size_t[CHUNK_SIZE])
            , m_chk((byte*) m_mem.get(), CHUNK_SIZE * OBJ_SIZE, OBJ_SIZE)
    {}

    std::unique_ptr<size_t[]> m_mem;
    plain_pool_chunk m_chk;
};

TEST_F(plain_pool_chunk_test, test_is_memory_available)
{
    ASSERT_TRUE(m_chk.memory_available());

    for (int i = 0; i < CHUNK_SIZE; ++i) {
        ASSERT_TRUE(m_chk.memory_available());
        size_t* ptr = (size_t*) m_chk.allocate(OBJ_SIZE);
        *ptr = 42;
    }

    ASSERT_FALSE(m_chk.memory_available());
}

TEST_F(plain_pool_chunk_test, test_contains)
{
    ASSERT_FALSE(m_chk.contains(nullptr));

    byte* ptr1 = m_chk.allocate(OBJ_SIZE);
    ASSERT_TRUE(m_chk.contains(ptr1));

    byte* ptr2 = ((byte*) m_mem.get()) - 1;
    ASSERT_FALSE(m_chk.contains(ptr2));

    byte* ptr3 = ((byte*) m_mem.get()) + OBJ_SIZE * CHUNK_SIZE;
    ASSERT_FALSE(m_chk.contains(ptr3));
}

TEST_F(plain_pool_chunk_test, test_empty)
{
    ASSERT_TRUE(m_chk.empty(OBJ_SIZE));

    for (int i = 0; i < CHUNK_SIZE; ++i) {
        size_t* ptr = (size_t*) m_chk.allocate(OBJ_SIZE);
        ASSERT_FALSE(m_chk.empty(OBJ_SIZE));
    }
}

TEST_F(plain_pool_chunk_test, test_allocate_1)
{
    size_t* ptr1 = (size_t*) m_chk.allocate(OBJ_SIZE);
    EXPECT_NE(nullptr, ptr1);
    // try dereference pointer
    // in case of incorrect allocation it might cause segmentation fault
    *ptr1 = 42;

    size_t* ptr2 = (size_t*) m_chk.allocate(OBJ_SIZE);
    EXPECT_NE(nullptr, ptr2);
    EXPECT_NE(ptr1, ptr2);
    *ptr2 = 42;
}

TEST_F(plain_pool_chunk_test, test_allocate_2)
{
    for (int i = 0; i < CHUNK_SIZE; ++i) {
        size_t* ptr = (size_t*) m_chk.allocate(OBJ_SIZE);
    }

    byte* ptr = m_chk.allocate(OBJ_SIZE);
    ASSERT_EQ(nullptr, ptr);
}

TEST_F(plain_pool_chunk_test, test_deallocate)
{
    size_t* ptr = nullptr;
    for (int i = 0; i < CHUNK_SIZE; ++i) {
        ptr = (size_t*) m_chk.allocate(OBJ_SIZE);
    }

    m_chk.deallocate((byte*) ptr, OBJ_SIZE);
    ASSERT_TRUE(m_chk.memory_available());

    ptr = (size_t*) m_chk.allocate(OBJ_SIZE);
    EXPECT_NE(nullptr, ptr);
    EXPECT_FALSE(m_chk.memory_available());
}