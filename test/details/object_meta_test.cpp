#include <gtest/gtest.h>

#include <vector>
#include <type_traits>

#include <libprecisegc/details/object_meta.hpp>

using namespace precisegc::details;

struct object_meta_test : public ::testing::Test
{
    struct test_type
    {
        void* data;
    };

    static const type_meta* tmeta;
};

const type_meta* object_meta_test::tmeta = type_meta_provider<test_type>::create_meta(std::vector<size_t>{1, 2, 3});

TEST_F(object_meta_test, test_constructor)
{
    object_meta ometa(tmeta);
    ASSERT_EQ(ometa.get_type_meta(), tmeta);
    ASSERT_EQ(ometa.type_size(), tmeta->type_size());
    ASSERT_EQ(ometa.offsets(), tmeta->offsets());
    ASSERT_EQ(0, ometa.object_count());
}

TEST_F(object_meta_test, test_object_count)
{
    object_meta ometa(tmeta);

    static const size_t OBJ_COUNT = 4;
    ometa.set_object_count(4);

    ASSERT_EQ(OBJ_COUNT, ometa.object_count());
    ASSERT_EQ(OBJ_COUNT * sizeof(test_type), ometa.object_size());
}

TEST_F(object_meta_test, test_forward_pointer)
{
    static const size_t STORAGE_SIZE = sizeof(test_type) + sizeof(object_meta);
    std::aligned_storage<STORAGE_SIZE> storage;
    byte* pstorage = reinterpret_cast<byte*>(&storage);
    test_type* pvalue = reinterpret_cast<test_type*>(pstorage);

    object_meta* ometa = object_meta::get_meta_ptr(pstorage, STORAGE_SIZE);
    new (ometa) object_meta(tmeta);
    ometa->set_object_count(1);

    byte* p;
    ometa->set_forward_pointer(p);
    ASSERT_EQ(p, ometa->forward_pointer());
    ASSERT_EQ(p, pvalue->data);
}

