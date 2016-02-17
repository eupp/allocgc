#include <gtest/gtest.h>

#include <cstdlib>
#include <cmath>
#include <atomic>
#include <queue>
#include <pthread.h>

#include "libprecisegc/thread.h"
#include "libprecisegc/gc_ptr.h"
#include "libprecisegc/gc_new.h"
#include "libprecisegc/gc.h"
#include "libprecisegc/details/barrier.h"
#include "libprecisegc/details/math_util.h"

using namespace precisegc;

static const int THREADS_COUNT = 4; // must be power of 2

namespace {

struct node
{
    gc_ptr<node> m_left;
    gc_ptr<node> m_right;
};

gc_ptr<node> create_gc_node()
{
    return gc_new<node>();
}

gc_ptr<node> create_tree()
{
    std::queue<gc_ptr<node>> q;
    for (int i = 0; i < THREADS_COUNT; ++i) {
        q.push(create_gc_node());
    }
    while (q.size() > 1) {
        auto parent = create_gc_node();
        parent->m_left = q.front();
        q.pop();
        parent->m_right = q.front();
        q.pop();
        q.push(parent);
    }
    return q.front();
}

void print_tree(gc_ptr<node>& root, const std::string& offset)
{
    gc_pin<node> pin(root);
    std::cout << offset << & root << " (" << pin.get() << ")" << std::endl;
    auto new_offset = offset + "    ";
    if (root->m_left) {
        print_tree(root->m_left, new_offset);
    } else {
        std::cout << new_offset << "nullptr" << std::endl;
    }
    if (root->m_right) {
        print_tree(root->m_right, new_offset);
    } else {
        std::cout << new_offset << "nullptr" << std::endl;
    }
}

std::atomic<int> thread_num(0);
std::atomic<bool> gc_finished(false);

static ::precisegc::details::barrier threads_ready(THREADS_COUNT + 1);

static void* thread_routine(void* arg)
{
    using namespace precisegc::details;

    gc_ptr<node>& root = * ((gc_ptr<node>*) arg);
    int num = thread_num++;
    // assign to each thread a leaf in the tree
    gc_ptr<node> ptr = root;
    for (int i = 0, k = 2; i < log_2(THREADS_COUNT); ++i, k *= 2) {
        ptr = num % k ? ptr->m_left : ptr->m_right;
    }
    threads_ready.wait();
    while (!gc_finished) {
        gc_ptr<node>& new_ptr = rand() % 2 ? ptr->m_left : ptr->m_right;
        new_ptr = create_gc_node();
    }
}

}

// This test doesn't check anything.
// Consider it is passed if nothing will crash or hang.
TEST(gc_test, test_gc)
{
    gc_ptr<node> root = create_tree();
//    print_tree(root, "");

    pthread_t threads[THREADS_COUNT];
    for (auto& thread: threads) {
        ASSERT_EQ(0, thread_create(&thread, nullptr, thread_routine, (void*) &root));
    }
    threads_ready.wait();

    root.reset();
    gc();
    gc_finished = true;

    for (auto& thread: threads) {
        void* ret = nullptr;
        thread_join(thread, &ret);
    }
}