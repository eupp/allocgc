#include <gtest/gtest.h>

#include "libprecisegc/details/managed_ptr.h"
#include "libprecisegc/details/mutex.h"

using namespace precisegc::details;

namespace {
typedef managed_cell_ptr::lock_type lock_type;
}

TEST(managed_ptr_test, test_default_constructor)
{
    managed_cell_ptr cell_ptr1;
    EXPECT_EQ(nullptr, cell_ptr1.get());

    managed_cell_ptr cell_ptr2(nullptr);
    EXPECT_EQ(nullptr, cell_ptr2.get());
}

TEST(managed_ptr_test, test_constructor)
{
    byte val = 0;
    managed_ptr mptr(&val);
    managed_cell_ptr cell_ptr(mptr);
    EXPECT_EQ(&val, cell_ptr.get());
}

TEST(managed_ptr_test, test_lock_constructor)
{
    mutex m;
    lock_type lock(m);
    byte val = 0;
    managed_ptr mptr(&val);
    managed_memory_descriptor* descr = nullptr;
    managed_cell_ptr cell_ptr(mptr, descr, std::move(lock));

    EXPECT_EQ(&val, cell_ptr.get());

    EXPECT_FALSE(lock.owns_lock());
    EXPECT_TRUE(cell_ptr.owns_descriptor_lock());
}