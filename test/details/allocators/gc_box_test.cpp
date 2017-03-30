#include <gtest/gtest.h>

#include <unordered_set>

#include <libprecisegc/details/allocators/gc_box.hpp>
#include <libprecisegc/gc_type_meta.hpp>

using namespace precisegc;
using namespace precisegc::details;
using namespace precisegc::details::allocators;

namespace {
struct test_type
{
    test_type()
        : m_ptr1(0)
        , m_ptr2(0)
    {}

    test_type(test_type&& other)
        : m_ptr1(other.m_ptr1)
        , m_ptr2(other.m_ptr2)
    {
        ++move_ctor_call_cnt;
    }

    ~test_type()
    {
        ++dtor_call_cnt;
    }

    static size_t dtor_call_cnt;
    static size_t move_ctor_call_cnt;

    static const gc_type_meta* type_meta;

    std::uintptr_t m_ptr1;
    std::uintptr_t m_ptr2;
};

size_t test_type::dtor_call_cnt = 0;
size_t test_type::move_ctor_call_cnt = 0;

const gc_type_meta* test_type::type_meta =
        gc_type_meta_factory<test_type>::create(std::vector<size_t>{0, sizeof(std::uintptr_t)});

}

struct gc_box_test : public ::testing::Test
{
    gc_box_test()
        : memory(new byte[STORAGE_SIZE])
        , cell_start(memory.get())
    {
        obj_start = gc_box::create(cell_start, OBJ_CNT, test_type::type_meta);
        obj1 = reinterpret_cast<test_type*>(obj_start);
        obj2 = reinterpret_cast<test_type*>(obj_start + sizeof(test_type));
        new (obj1) test_type();
        new (obj2) test_type();
    }

    static const size_t OBJ_CNT = 2;
    static const size_t STORAGE_SIZE = gc_box::box_size(OBJ_CNT * sizeof(test_type));

    std::unique_ptr<byte[]> memory;
    byte* cell_start;
    byte* obj_start;
    test_type* obj1;
    test_type* obj2;
};

TEST_F(gc_box_test, test_create)
{
    EXPECT_EQ((size_t) OBJ_CNT, gc_box::get_obj_count(cell_start));
    EXPECT_EQ(test_type::type_meta, gc_box::get_type_meta(cell_start));
    EXPECT_EQ(0, obj1->m_ptr1);
    EXPECT_EQ(0, obj1->m_ptr2);
    EXPECT_EQ(0, obj2->m_ptr2);
    EXPECT_EQ(0, obj2->m_ptr2);
    EXPECT_FALSE(gc_box::is_forwarded(cell_start));
}

TEST_F(gc_box_test, test_trace)
{
    std::unordered_set<byte*> expected_set;
    byte* it = obj_start;
    for (size_t i = 0; i < 2 * OBJ_CNT; ++i, it += sizeof(std::uintptr_t)) {
        expected_set.insert(it);
    }

    std::unordered_set<byte*> traced_set;
    gc_box::trace(cell_start, gc_trace_callback{[&traced_set] (gc_handle* handle) {
        traced_set.insert(reinterpret_cast<byte*>(handle));
    }});

    ASSERT_EQ(expected_set, traced_set);
}

TEST_F(gc_box_test, test_move)
{
    std::unique_ptr<byte[]> to_storage(new byte[STORAGE_SIZE]);
    byte* to = to_storage.get();
    test_type* to_obj1 = reinterpret_cast<test_type*>(gc_box::get_obj_start(to));
    test_type* to_obj2 = reinterpret_cast<test_type*>(gc_box::get_obj_start(to) + sizeof(test_type));

    gc_box::move(to, cell_start, OBJ_CNT, test_type::type_meta);

    EXPECT_EQ((size_t) OBJ_CNT, gc_box::get_obj_count(to));
    EXPECT_EQ(test_type::type_meta, gc_box::get_type_meta(to));
    EXPECT_EQ(0, to_obj1->m_ptr1);
    EXPECT_EQ(0, to_obj1->m_ptr2);
    EXPECT_EQ(0, to_obj2->m_ptr1);
    EXPECT_EQ(0, to_obj2->m_ptr2);
    EXPECT_EQ(2, test_type::move_ctor_call_cnt);
    EXPECT_FALSE(gc_box::is_forwarded(to));
}

TEST_F(gc_box_test, test_destroy)
{
    gc_box::destroy(cell_start);

    EXPECT_EQ(2, test_type::dtor_call_cnt);
}

TEST_F(gc_box_test, test_forward)
{
    std::aligned_storage<STORAGE_SIZE> to_storage;
    byte* forward_pointer = reinterpret_cast<byte*>(&to_storage);
    gc_box::set_forward_pointer(cell_start, forward_pointer);

    ASSERT_TRUE(gc_box::is_forwarded(cell_start));
    ASSERT_EQ(gc_box::get_obj_start(forward_pointer), gc_box::forward_pointer(cell_start));
}