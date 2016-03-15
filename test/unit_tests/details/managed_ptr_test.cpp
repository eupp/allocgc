#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "libprecisegc/details/managed_ptr.h"
#include "libprecisegc/details/mutex.h"

#include "memory_descriptor_mock.h"
#include "page_ptr.h"

using namespace precisegc::details;

using ::testing::Exactly;

namespace {
typedef managed_cell_ptr::lock_type lock_type;
}

struct managed_cell_ptr_test : public ::testing::Test
{
    managed_cell_ptr_test()
        : m_ptr(managed_ptr::index(m_mem.get(), PAGE_SIZE, &m_mock))
    {}

    ~managed_cell_ptr_test()
    {
        managed_ptr::remove_index(m_ptr, PAGE_SIZE);
    }

    page_ptr m_mem;
    memory_descriptor_mock m_mock;
    managed_ptr m_ptr;
};


TEST_F(managed_cell_ptr_test, test_default_constructor)
{
    managed_cell_ptr cell_ptr1;
    EXPECT_EQ(nullptr, cell_ptr1.get());

    managed_cell_ptr cell_ptr2(nullptr);
    EXPECT_EQ(nullptr, cell_ptr2.get());
}

TEST_F(managed_cell_ptr_test, test_constructor)
{
    managed_cell_ptr cell_ptr1(m_ptr);
    EXPECT_EQ(m_mem.get(), cell_ptr1.get());

    managed_cell_ptr cell_ptr2(m_ptr, &m_mock);
    EXPECT_EQ(m_mem.get(), cell_ptr2.get());
}

TEST_F(managed_cell_ptr_test, test_lock_constructor)
{
    mutex m;
    lock_type lock(m);
    managed_cell_ptr cell_ptr(m_ptr, &m_mock, std::move(lock));

    EXPECT_EQ(m_mem.get(), cell_ptr.get());

    EXPECT_FALSE(lock.owns_lock());
    EXPECT_TRUE(cell_ptr.owns_descriptor_lock());
}

TEST_F(managed_cell_ptr_test, test_get_mark)
{
    managed_cell_ptr cell_ptr(m_ptr, &m_mock);

    EXPECT_CALL(m_mock, get_mark(m_mem.get()))
        .Times(Exactly(1));

    cell_ptr.get_mark();
}

TEST_F(managed_cell_ptr_test, test_get_pin)
{
    managed_cell_ptr cell_ptr(m_ptr, &m_mock);

    EXPECT_CALL(m_mock, get_pin(m_mem.get()))
            .Times(Exactly(1));

    cell_ptr.get_pin();
}

TEST_F(managed_cell_ptr_test, test_set_mark)
{
    managed_cell_ptr cell_ptr(m_ptr, &m_mock);

    EXPECT_CALL(m_mock, set_mark(m_mem.get(), true))
            .Times(Exactly(1));

    cell_ptr.set_mark(true);
}

TEST_F(managed_cell_ptr_test, test_set_pin)
{
    managed_cell_ptr cell_ptr(m_ptr, &m_mock);

    EXPECT_CALL(m_mock, set_pin(m_mem.get(), true))
            .Times(Exactly(1));

    cell_ptr.set_pin(true);
}

TEST_F(managed_cell_ptr_test, test_sweep)
{
    managed_cell_ptr cell_ptr(m_ptr, &m_mock);

    EXPECT_CALL(m_mock, sweep(m_mem.get()))
            .Times(Exactly(1));

    cell_ptr.sweep();
}

TEST_F(managed_cell_ptr_test, test_get_meta)
{
    managed_cell_ptr cell_ptr(m_ptr, &m_mock);

    EXPECT_CALL(m_mock, get_cell_meta(m_mem.get()))
            .Times(Exactly(1));

    cell_ptr.get_meta();
}

TEST_F(managed_cell_ptr_test, test_descriptor_lazy_init_1)
{
    managed_cell_ptr cell_ptr1(m_ptr);
    managed_cell_ptr cell_ptr2(m_ptr);
    managed_cell_ptr cell_ptr3(m_ptr);
    managed_cell_ptr cell_ptr4(m_ptr);
    managed_cell_ptr cell_ptr5(m_ptr);
    managed_cell_ptr cell_ptr6(m_ptr);

    EXPECT_CALL(m_mock, get_mark(m_mem.get()))
            .Times(Exactly(1));
    EXPECT_CALL(m_mock, get_pin(m_mem.get()))
            .Times(Exactly(1));
    EXPECT_CALL(m_mock, set_mark(m_mem.get(), true))
            .Times(Exactly(1));
    EXPECT_CALL(m_mock, set_pin(m_mem.get(), true))
            .Times(Exactly(1));
    EXPECT_CALL(m_mock, get_cell_meta(m_mem.get()))
            .Times(Exactly(1));
    EXPECT_CALL(m_mock, sweep(m_mem.get()))
            .Times(Exactly(1));


    cell_ptr1.get_mark();
    cell_ptr2.get_pin();
    cell_ptr3.set_mark(true);
    cell_ptr4.set_pin(true);
    cell_ptr5.get_meta();
    cell_ptr6.sweep();
}

TEST_F(managed_cell_ptr_test, test_descriptor_lazy_init_2)
{
    byte val = 0;
    managed_ptr ptr(&val);
    managed_cell_ptr cell_ptr(ptr);

    EXPECT_THROW(cell_ptr.get_meta(), managed_cell_ptr::unindexed_memory_exception);
}