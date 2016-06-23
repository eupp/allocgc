#include <gtest/gtest.h>

#include <libprecisegc/details/utils/base_offset.hpp>

using namespace precisegc::details::utils;

namespace {

struct Base {
    int b;
};

struct Derived : public Base {
    double d;
};

}

TEST(base_offset_test, test_inheritance)
{
    Derived d;
    ASSERT_EQ(0, base_offset<Base>(&d));
}

namespace {

struct EmptyBase {};

struct DerivedFromEmpty : public EmptyBase {
    int d;
};

}

TEST(base_offset_test, test_empty_base)
{
    DerivedFromEmpty d;
    ASSERT_EQ(0, base_offset<EmptyBase>(&d));
}

namespace {

struct Base1 {
    int b1;
};

struct Base2 {
};

struct Base3 {
    double b3;
};

struct DerivedFromMultipleBases : public Base1, public Base2, public Base3 {
    short d;
};

}

TEST(base_offset_test, test_multiple_inheritance)
{
    DerivedFromMultipleBases d;

    ASSERT_EQ(0, base_offset<Base1>(&d));
    ASSERT_EQ(sizeof(Base1), base_offset<Base2>(&d));
    ASSERT_EQ(sizeof(Base1) + sizeof(Base2), base_offset<Base3>(&d));
}