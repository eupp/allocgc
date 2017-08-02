#include <gtest/gtest.h>

#include <memory>
#include <algorithm>

#include <liballocgc/gc_common.hpp>
#include <liballocgc/details/allocators/freelist_pool_chunk.hpp>

#include <boost/integer/static_min_max.hpp>

using namespace allocgc;
using namespace allocgc::details;
using namespace allocgc::details::allocators;

template <typename Chunk>
struct pool_chunk_test : public ::testing::Test
{
    static const int CHUNK_SIZE = boost::static_unsigned_min<(size_t) 64, Chunk::CHUNK_MAXSIZE>::value;
    static const int OBJ_SIZE = sizeof(size_t);

    pool_chunk_test()
        : m_mem(new size_t[CHUNK_SIZE])
        , m_chk((byte*) m_mem.get(), CHUNK_SIZE * OBJ_SIZE, OBJ_SIZE)
    {}

    std::unique_ptr<size_t[]> m_mem;
    Chunk m_chk;
};

typedef ::testing::Types<freelist_pool_chunk> test_chunk_types;
TYPED_TEST_CASE(pool_chunk_test, test_chunk_types);

TYPED_TEST(pool_chunk_test, test_memory_available)
{
    ASSERT_TRUE(this->m_chk.memory_available());

    for (int i = 0; i < this->CHUNK_SIZE; ++i) {
        ASSERT_TRUE(this->m_chk.memory_available());
        size_t* ptr = (size_t*) this->m_chk.allocate(this->OBJ_SIZE);
        *ptr = 42;
    }

    ASSERT_FALSE(this->m_chk.memory_available());
}

TYPED_TEST(pool_chunk_test, test_contains)
{
    ASSERT_FALSE(this->m_chk.contains(nullptr));

    byte* ptr1 = this->m_chk.allocate(this->OBJ_SIZE);
    ASSERT_TRUE(this->m_chk.contains(ptr1));

    byte* ptr2 = ((byte*) this->m_mem.get()) - 1;
    ASSERT_FALSE(this->m_chk.contains(ptr2));

    byte* ptr3 = ((byte*) this->m_mem.get()) + this->OBJ_SIZE * this->CHUNK_SIZE;
    ASSERT_FALSE(this->m_chk.contains(ptr3));
}

TYPED_TEST(pool_chunk_test, test_empty)
{
    ASSERT_TRUE(this->m_chk.empty());

    for (int i = 0; i < this->CHUNK_SIZE; ++i) {
        size_t* ptr = (size_t*) this->m_chk.allocate(this->OBJ_SIZE);
        ASSERT_FALSE(this->m_chk.empty());
    }
}

TYPED_TEST(pool_chunk_test, test_allocate)
{
    size_t* ptr1 = (size_t*) this->m_chk.allocate(this->OBJ_SIZE);
    EXPECT_NE(nullptr, ptr1);
    // try dereference pointer
    // in case of incorrect allocation it might cause segmentation fault
    *ptr1 = 42;

    size_t* ptr2 = (size_t*) this->m_chk.allocate(this->OBJ_SIZE);
    EXPECT_NE(nullptr, ptr2);
    EXPECT_NE(ptr1, ptr2);
    *ptr2 = 42;
}

TYPED_TEST(pool_chunk_test, test_deallocate)
{
    size_t* ptr = nullptr;
    for (int i = 0; i < this->CHUNK_SIZE; ++i) {
        ptr = (size_t*) this->m_chk.allocate(this->OBJ_SIZE);
    }

    this->m_chk.deallocate((byte*) ptr, this->OBJ_SIZE);
    ASSERT_TRUE(this->m_chk.memory_available());

    ptr = (size_t*) this->m_chk.allocate(this->OBJ_SIZE);
    EXPECT_NE(nullptr, ptr);
    EXPECT_FALSE(this->m_chk.memory_available());
}