#include <gtest/gtest.h>

#include <memory>

#include "libprecisegc/details/segregated_list.h"
#include "libprecisegc/object.h"

using namespace std;
using namespace precisegc::details;

static const size_t OBJ_SIZE = 8;

TEST(test_segregated_list_element, test_allocate)
{
    unique_ptr<segregated_list_element> sle(new segregated_list_element(OBJ_SIZE));
    for (int i = 0; i < PAGES_PER_SEGREGATED_STORAGE_ELEMENT; ++i) {
        ASSERT_TRUE(sle->is_memory_available());
        auto alloc_res = sle->allocate();
        page_descriptor* pd = alloc_res.second;
        for (int j = 0; j < pd->page_size() / OBJ_SIZE - 1; ++j) {
            ASSERT_TRUE(sle->is_memory_available());
            auto alloc_res = sle->allocate();
            size_t* ptr = (size_t*) alloc_res.first;
            EXPECT_NE(nullptr, ptr);
            EXPECT_EQ(pd, alloc_res.second);
            *ptr = 42;
        }
    }
    ASSERT_FALSE(sle->is_memory_available());
}

TEST(test_segregated_list_element, test_clear)
{
    unique_ptr<segregated_list_element> sle(new segregated_list_element(OBJ_SIZE));
    size_t total_cnt = 0;
    for (int i = 0; i < PAGES_PER_SEGREGATED_STORAGE_ELEMENT; ++i) {
        auto alloc_res = sle->allocate();
        page_descriptor* pd = alloc_res.second;
        total_cnt = pd->page_size() / OBJ_SIZE;
        for (int j = 0; j < pd->page_size() / OBJ_SIZE - 1; ++j) {
            sle->allocate();
        }
    }

    auto& pd1 = sle->get_page_descriptor(LAST_PAGE_ID);
    sle->clear(pd1.end(), LAST_PAGE_ID);
    ASSERT_EQ(LAST_PAGE_ID, sle->last_used_page());
    ASSERT_EQ(pd1.end(), sle->get_page_descriptor(LAST_PAGE_ID).end());

    size_t page_id = LAST_PAGE_ID - 1;
    auto& pd2 = sle->get_page_descriptor(page_id);
    page_descriptor::iterator it = pd2.begin();
    sle->clear(pd2.begin(), page_id);
    ASSERT_EQ(page_id, sle->last_used_page());
    ASSERT_EQ(pd2.begin(), sle->get_page_descriptor(page_id).end());

    auto& pd3 = sle->get_page_descriptor(0);
    sle->clear(pd3.begin(), 0);
    ASSERT_EQ(0, sle->last_used_page());
    ASSERT_EQ(pd3.begin(), sle->get_page_descriptor(0).end());
}

// fill one segregated_list_element and returns count of objects allocated
size_t allocate_on_one_sle(segregated_list& sl)
{
    size_t total_cnt = 0;
    for (int i = 0 ; i < PAGES_PER_SEGREGATED_STORAGE_ELEMENT; ++i) {
        auto alloc_res = sl.allocate();
        page_descriptor* pd = alloc_res.second;
        total_cnt += pd->page_size() / OBJ_SIZE;
        for (int j = 0; j < pd->page_size() / OBJ_SIZE - 1; ++j) {
            sl.allocate();
        }
    }
    return total_cnt;
}

TEST(test_segregated_list, test_allocate)
{
    segregated_list sl(OBJ_SIZE);
    // allocate at least on two sle
    allocate_on_one_sle(sl);
    auto alloc_res = sl.allocate();
    size_t* ptr = (size_t*) alloc_res.first;
    ASSERT_NE(nullptr, ptr);
    ASSERT_NE(nullptr, alloc_res.second);
    *ptr = 42;
}

TEST(test_segregated_list, test_empty_iterators)
{
    segregated_list sl(OBJ_SIZE);
    ASSERT_EQ(sl.begin(), sl.end());
}

TEST(test_segregated_list, test_iterators)
{
    segregated_list sl(OBJ_SIZE);
    // allocate at least on two sle
    size_t total_cnt = allocate_on_one_sle(sl);
    auto alloc_res = sl.allocate();
    total_cnt += 1;

    size_t cnt = 0;
    for (auto it = sl.begin(); it != sl.end(); ++it) {
        ++cnt;
        size_t* ptr = (size_t*) *it;
        ASSERT_NE(nullptr, ptr);
        *ptr = 42;
    }
    ASSERT_EQ(total_cnt, cnt);
}

TEST(test_segregated_list, test_iterators_reverse)
{
    segregated_list sl(OBJ_SIZE);
    // allocate at least on two sle
    size_t total_cnt = allocate_on_one_sle(sl);
    auto alloc_res = sl.allocate();
    total_cnt += 1;

    size_t cnt = 0;
    for (auto it = sl.end(); it != sl.begin(); --it) {
        ++cnt;
        if (it != sl.end()) {
            size_t* ptr = (size_t*) *it;
            ASSERT_NE(nullptr, ptr);
            *ptr = 42;
        }
    }
    ASSERT_EQ(total_cnt, cnt);
}

TEST(test_segregated_list, test_clear)
{
    segregated_list sl(OBJ_SIZE);
    // allocate at least on two sle
    size_t total_cnt = allocate_on_one_sle(sl);
    auto alloc_res = sl.allocate();
    total_cnt += 1;

    auto end = sl.end();
    auto it = sl.begin();
    std::advance(it, total_cnt);
    sl.clear(it);
    ASSERT_EQ(end, sl.end());

    it = sl.begin();
    std::advance(it, total_cnt - 1);
    sl.clear(it);
    ASSERT_EQ(it, sl.end());

    it = sl.begin();
    std::advance(it, 1);
    sl.clear(it);
    ASSERT_EQ(it, sl.end());

    it = sl.begin();
    sl.clear(it);
    ASSERT_EQ(sl.end(), sl.begin());
}

TEST(test_segregated_list, test_compact)
{
    segregated_list sl(OBJ_SIZE);
    // allocate at least on two sle
    size_t total_cnt = allocate_on_one_sle(sl);
    sl.allocate();
    total_cnt += 1;
    ASSERT_TRUE(total_cnt > 8);

    // mark & pin some objects
    auto it1 = sl.begin();
    it1.set_marked(true);
    advance(it1, 1);
    void* exp_to = *it1;

    auto it2 = sl.begin();
    std::advance(it2, 3);
    it2.set_marked(true);
    it2.set_pinned(true);

    auto it3 = sl.begin();
    std::advance(it3, 4);
    void* exp_from = *it3;
    it3.set_marked(true);

    forwarding_list fl;
    sl.compact(fl);
    auto end = it2;
    ++end;
    EXPECT_EQ(end, sl.end());

    void* from = fl[0].from();
    void* to = fl[0].to();

    EXPECT_EQ(1, fl.size());
    EXPECT_EQ(exp_from, from);
    EXPECT_EQ(exp_to, to);
}

TEST(segregated_list_test, test_forwarding)
{
    size_t alloc_size = sizeof(void*) + sizeof(Object);
    segregated_list sl(alloc_size);
    void* ptr = sl.allocate().first;

    size_t meta[3];
    meta[0] = sizeof(void*);
    meta[1] = 1;
    meta[2] = 0;

    void*& from = * (void**) ptr;
    Object* obj = (Object*) ((size_t) ptr + sizeof(void*));
    obj->meta = (void*) meta;
    obj->count = 1;
    obj->begin = from;

    forwarding_list forwarding;
    forwarding.emplace_back(from, nullptr);

    sl.fix_pointers(forwarding);

    ASSERT_EQ(nullptr, from);
}