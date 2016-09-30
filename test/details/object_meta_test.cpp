#include <gtest/gtest.h>

#include <vector>
#include <type_traits>

#include <libprecisegc/details/object_meta.hpp>

using namespace precisegc::details;

struct object_meta_test : public ::testing::Test
{
    struct test_type
    {
        byte* data;
    };

    static const type_meta* tmeta;
};

const type_meta* object_meta_test::tmeta = type_meta_provider<test_type>::create_meta(std::vector<size_t>{1, 2, 3});

TEST_F(object_meta_test, test_get_meta_ptr)
{
    static const size_t STORAGE_SIZE = sizeof(test_type) + sizeof(object_meta);
    std::aligned_storage<STORAGE_SIZE> storage;

    object_meta* meta_ptr = object_meta::get_meta_ptr((byte*) &storage, STORAGE_SIZE);
    byte* obj_ptr = object_meta::get_object_ptr((byte*) &storage, STORAGE_SIZE);

    ASSERT_NE(nullptr, meta_ptr);
    ASSERT_NE(nullptr, obj_ptr);
    ASSERT_NE((byte*) meta_ptr, obj_ptr);
}

TEST_F(object_meta_test, test_constructor)
{
    object_meta obj_meta(0, tmeta);
    ASSERT_EQ(obj_meta.get_type_meta(), tmeta);
    ASSERT_EQ(obj_meta.type_size(), tmeta->type_size());
    ASSERT_EQ(obj_meta.offsets(), tmeta->offsets());
    ASSERT_EQ(0, obj_meta.object_count());
}

TEST_F(object_meta_test, test_object_count)
{
    object_meta obj_meta(0, tmeta);

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
    test_type* pvalue = reinterpret_cast<test_type*>(object_meta::get_object_ptr((byte*) &pstorage, STORAGE_SIZE));
    memset(&pvalue->data, 0, sizeof(void*));

    object_meta* obj_meta = object_meta::get_meta_ptr(pstorage, STORAGE_SIZE);
    new (obj_meta) object_meta(0, tmeta);
    obj_meta->set_object_count(1);

    byte* p;
    obj_meta->set_forward_pointer(p);
    ASSERT_EQ(p, obj_meta->forward_pointer());
//    ASSERT_EQ(p, (byte*) pvalue->data);
}

TEST_F(object_meta_test, test_get_object_begin)
{
    static const size_t STORAGE_SIZE = sizeof(test_type) + sizeof(object_meta);
    std::aligned_storage<STORAGE_SIZE> storage;

    object_meta* meta_ptr = object_meta::get_meta_ptr((byte*) &storage, STORAGE_SIZE);
    byte* obj_ptr = object_meta::get_object_ptr((byte*) &storage, STORAGE_SIZE);

    ASSERT_EQ(obj_ptr, meta_ptr->get_object_begin());
}

TEST_F(object_meta_test, test_get_array_element_begin)
{
    static const size_t OBJ_SIZE = sizeof(test_type);
    static const size_t ARRAY_SIZE = 8;
    static const size_t STORAGE_SIZE = ARRAY_SIZE * sizeof(test_type) + sizeof(object_meta);

    std::aligned_storage<STORAGE_SIZE> storage;
    object_meta* meta = object_meta::get_meta_ptr((byte*) &storage, STORAGE_SIZE);
    byte* array = object_meta::get_object_ptr((byte*) &storage, STORAGE_SIZE);

    type_meta_provider<test_type>::create_meta();
    const type_meta* cls_meta = type_meta_provider<test_type>::get_meta();
    new (meta) object_meta(ARRAY_SIZE, cls_meta);

    for (size_t i = 0; i < ARRAY_SIZE; ++i) {
        byte* obj_begin = array + i * OBJ_SIZE;
        for (byte* p = obj_begin; p < obj_begin + OBJ_SIZE; ++p) {
            ASSERT_EQ(obj_begin, meta->get_array_element_begin(p));
        }
    }
}
