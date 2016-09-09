#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "libprecisegc/details/managed_ptr.hpp"

#include "memory_descriptor_mock.h"
#include "page_ptr.h"

using namespace precisegc::details;

using ::testing::Exactly;

struct managed_ptr_test : public ::testing::Test
{
    managed_ptr_test()
        : m_mem(make_page_ptr())
    {
        managed_ptr::add_to_index(m_mem.get(), PAGE_SIZE, &m_mock);
    }

    ~managed_ptr_test()
    {
        managed_ptr::remove_from_index(m_mem.get(), PAGE_SIZE);
    }

    page_ptr m_mem;
    memory_descriptor_mock m_mock;
};

TEST_F(managed_ptr_test, test_default_constructor)
{
    managed_ptr ptr1;
    EXPECT_EQ(nullptr, ptr1.get());

    managed_ptr ptr2(nullptr);
    EXPECT_EQ(nullptr, ptr2.get());
}

TEST_F(managed_ptr_test, test_constructor)
{
    managed_ptr ptr1(m_mem.get());
    EXPECT_EQ(m_mem.get(), ptr1.get());

    managed_ptr ptr2(m_mem.get(), &m_mock);
    EXPECT_EQ(m_mem.get(), ptr2.get());
}

TEST_F(managed_ptr_test, test_get_mark)
{
    managed_ptr ptr(m_mem.get(), &m_mock);

    EXPECT_CALL(m_mock, get_mark(m_mem.get()))
        .Times(Exactly(1));

    ptr.get_mark();
}

TEST_F(managed_ptr_test, test_get_pin)
{
    managed_ptr ptr(m_mem.get(), &m_mock);

    EXPECT_CALL(m_mock, get_pin(m_mem.get()))
            .Times(Exactly(1));

    ptr.get_pin();
}

TEST_F(managed_ptr_test, test_set_mark)
{
    managed_ptr ptr(m_mem.get(), &m_mock);

    EXPECT_CALL(m_mock, set_mark(m_mem.get(), true))
            .Times(Exactly(1));

    ptr.set_mark(true);
}

TEST_F(managed_ptr_test, test_set_pin)
{
    managed_ptr ptr(m_mem.get(), &m_mock);

    EXPECT_CALL(m_mock, set_pin(m_mem.get(), true))
            .Times(Exactly(1));

    ptr.set_pin(true);
}

TEST_F(managed_ptr_test, test_cell_size)
{
    managed_ptr ptr(m_mem.get(), &m_mock);

    EXPECT_CALL(m_mock, cell_size())
            .Times(Exactly(1));

    ptr.cell_size();
}

TEST_F(managed_ptr_test, test_get_meta)
{
    managed_ptr ptr(m_mem.get(), &m_mock);

    EXPECT_CALL(m_mock, get_cell_meta(m_mem.get()))
            .Times(Exactly(1));

    ptr.get_meta();
}

TEST_F(managed_ptr_test, test_get_cell_begin)
{
    managed_ptr ptr(m_mem.get(), &m_mock);

    EXPECT_CALL(m_mock, get_cell_begin(m_mem.get()))
            .Times(Exactly(1));

    ptr.get_cell_begin();
}

TEST_F(managed_ptr_test, test_get_obj_begin)
{
    managed_ptr ptr(m_mem.get(), &m_mock);

    EXPECT_CALL(m_mock, get_obj_begin(m_mem.get()))
            .Times(Exactly(1));

    ptr.get_obj_begin();
}