#include <gtest/gtest.h>

#include "libprecisegc/details/gc_compact.h"
#include "libprecisegc/details/segregated_list.h"

using namespace precisegc::details;

static const size_t OBJ_SIZE = PAGE_SIZE / OBJECTS_PER_PAGE;
static const size_t OBJ_COUNT = 5;

TEST(gc_compact_test, test_two_finger_compact)
{
    segregated_list sl(OBJ_SIZE);
    for (int i = 0; i < OBJ_COUNT; ++i) {
        auto alloc_res = sl.allocate();
        set_object_mark(alloc_res.first, false);
    }

    // mark & pin some objects
    auto it1 = sl.begin();
    it1.set_marked(true);
    ++it1;
    void* exp_to = *it1;

    auto it2 = std::next(sl.begin(), 3);
    it2.set_marked(true);
    it2.set_pinned(true);

    auto it3 = std::next(sl.begin(), 4);
    void* exp_from = *it3;
    it3.set_marked(true);

    forwarding_list frwd;
    two_finger_compact(sl.begin(), sl.end(), OBJ_SIZE, frwd);
    
    auto end = it2;
    ++end;
    ASSERT_EQ(end, sl.end());

    ASSERT_EQ(1, frwd.size());
    void* from = frwd[0].from();
    void* to = frwd[0].to();

    ASSERT_EQ(exp_from, from);
    ASSERT_EQ(exp_to, to);
}

struct test_type
{
    size_t val;
};

TEST(gc_compact_test, test_fix_pointers)
{
    segregated_list sl(OBJ_SIZE);
    auto alloc_res = sl.allocate();
    void* ptr = alloc_res.first;
    page_descriptor* pd = alloc_res.second;

    auto offsets = std::vector<size_t>({0});
    typedef class_meta_provider<test_type> provider;
    provider::create_meta(offsets);

    void*& from = * (void**) ptr;
    object_meta* obj_meta = object_meta::get_meta_ptr(ptr, pd->obj_size());
    obj_meta->set_class_meta(provider::get_meta_ptr());
    obj_meta->set_count(1);
    obj_meta->set_object_ptr(ptr);

    forwarding_list forwarding;
    forwarding.emplace_back(from, nullptr, OBJ_SIZE);

    fix_pointers(sl.begin(), sl.end(), OBJ_SIZE, forwarding);

    ASSERT_EQ(nullptr, from);
}