#include <gtest/gtest.h>

#include "libprecisegc/details/gc_compact.h"
#include "libprecisegc/details/segregated_list.h"

using namespace precisegc::details;

static const size_t OBJ_SIZE = MEMORY_CELL_SIZE / OBJECTS_PER_PAGE;
static const size_t OBJ_COUNT = 5;

TEST(gc_compact_test, test_two_finger_compact)
{
    segregated_list sl(OBJ_SIZE);
    for (int i = 0; i < OBJ_COUNT; ++i) {
        sl.allocate();
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

TEST(gc_compact_test, test_fix_pointers)
{
    segregated_list sl(OBJ_SIZE);
    auto alloc_res = sl.allocate();
    void* ptr = alloc_res.first;
    page_descriptor* pd = alloc_res.second;

    size_t meta[3];
    meta[0] = OBJ_SIZE;
    meta[1] = 1;
    meta[2] = 0;

    void*& from = * (void**) ptr;
    Object* obj = (Object*) ((size_t) ptr + pd->obj_size() - sizeof(Object));
    obj->meta = (void*) meta;
    obj->count = 1;
    obj->begin = ptr;

    forwarding_list forwarding;
    forwarding.emplace_back(from, nullptr, OBJ_SIZE);

    fix_pointers(sl.begin(), sl.end(), OBJ_SIZE, forwarding);

    ASSERT_EQ(nullptr, from);
}