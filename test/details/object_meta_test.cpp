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
    object_meta obj_meta(tmeta);
    ASSERT_EQ(obj_meta.get_type_meta(), tmeta);
    ASSERT_EQ(obj_meta.type_size(), tmeta->type_size());
    ASSERT_EQ(obj_meta.offsets(), tmeta->offsets());
    ASSERT_EQ(0, obj_meta.object_count());
}

TEST_F(object_meta_test, test_object_count)
{
    object_meta obj_meta(tmeta);

    static const size_t OBJ_COUNT = 4;
    obj_meta.set_object_count(4);

    ASSERT_EQ(OBJ_COUNT, obj_meta.object_count());
    ASSERT_EQ(OBJ_COUNT * sizeof(test_type), obj_meta.object_size());
}

TEST_F(object_meta_test, test_forward_pointer)
{
    static const size_t STORAGE_SIZE = sizeof(test_type) + sizeof(object_meta);
    std::aligned_storage<STORAGE_SIZE> storage;
    byte* pstorage = reinterpret_cast<byte*>(&storage);
    test_type* pvalue = reinterpret_cast<test_type*>(pstorage);
    memset(&pvalue->data, 0, sizeof(void*));

    object_meta* obj_meta = object_meta::get_meta_ptr(pstorage, STORAGE_SIZE);
    new (obj_meta) object_meta(tmeta);
    obj_meta->set_object_count(1);

    byte* p;
    obj_meta->set_forward_pointer(p);
    ASSERT_EQ(p, obj_meta->forward_pointer());
    ASSERT_EQ(p, (byte*) pvalue->data);
}

