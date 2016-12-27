#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <libprecisegc/details/collectors/memory_index.hpp>

#include "memory_descriptor_mock.h"
#include "page_ptr.h"

using namespace precisegc::details;
using namespace precisegc::details::collectors;

using ::testing::Exactly;

struct indexed_managed_object_test : public ::testing::Test
{
    indexed_managed_object_test()
        : m_mem(make_page_ptr())
        , m_obj_ptr(managed_object::get_object(m_mem.get()))
    {
        memory_index::add_to_index(m_mem.get(), PAGE_SIZE, &m_mock);
    }

    ~indexed_managed_object_test()
    {
        memory_index::remove_from_index(m_mem.get(), PAGE_SIZE);
    }

    page_ptr m_mem;
    byte* m_obj_ptr;
    memory_descriptor_mock m_mock;
};

TEST_F(indexed_managed_object_test, test_index)
{
    auto idx_obj = indexed_managed_object::index(m_obj_ptr);
    EXPECT_EQ(m_obj_ptr, idx_obj.object());
    EXPECT_EQ(&m_mock, idx_obj.descriptor());
}

TEST_F(indexed_managed_object_test, test_get_mark)
{
    indexed_managed_object ptr(m_obj_ptr, &m_mock);

    EXPECT_CALL(m_mock, get_mark(m_obj_ptr))
        .Times(Exactly(1));

    ptr.get_mark();
}

TEST_F(indexed_managed_object_test, test_get_pin)
{
    auto idx_obj = indexed_managed_object::index(m_obj_ptr);

    EXPECT_CALL(m_mock, get_pin(m_obj_ptr))
            .Times(Exactly(1));

    idx_obj.get_pin();
}

TEST_F(indexed_managed_object_test, test_set_mark)
{
    auto idx_obj = indexed_managed_object::index(m_obj_ptr);

    EXPECT_CALL(m_mock, set_mark(m_obj_ptr, true))
            .Times(Exactly(1));

    idx_obj.set_mark(true);
}

TEST_F(indexed_managed_object_test, test_set_pin)
{
    auto idx_obj = indexed_managed_object::index(m_obj_ptr);

    EXPECT_CALL(m_mock, set_pin(m_obj_ptr, true))
            .Times(Exactly(1));

    idx_obj.set_pin(true);
}

TEST_F(indexed_managed_object_test, test_cell_size)
{
    auto idx_obj = indexed_managed_object::index(m_obj_ptr);

    EXPECT_CALL(m_mock, cell_size(m_obj_ptr))
            .Times(Exactly(1));

    idx_obj.cell_size();
}