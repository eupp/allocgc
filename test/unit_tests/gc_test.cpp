#include <gtest/gtest.h>

#include <cstdlib>
#include <cmath>
#include <cassert>
#include <atomic>
#include <queue>
#include <pthread.h>

#include "libprecisegc/gc_ptr.h"
#include "libprecisegc/gc_new.h"
#include "libprecisegc/gc.h"
#include "libprecisegc/details/gc_mark.h"
#include "libprecisegc/details/barrier.h"
#include "libprecisegc/details/math_util.h"

using namespace precisegc;
using namespace precisegc::details;

//#define DEBUG_PRINT_TREE

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

gc_ptr<node> create_tree(size_t depth)
{
    std::queue<gc_ptr<node>> q;
    const size_t LEAFS_CNT = pow(2, depth);
    for (int i = 0; i < LEAFS_CNT; ++i) {
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

void unmark_tree(const gc_ptr<node>& ptr)
{
    if (ptr) {
        gc_pin<node> pin(ptr);
        set_object_mark(pin.get(), false);
        unmark_tree(ptr->m_left);
        unmark_tree(ptr->m_right);
    }
}

void print_tree(const gc_ptr<node>& root, const std::string& offset = "")
{
    #ifdef DEBUG_PRINT_TREE
        if (!root) {
            std::cout << offset << "nullptr" << std::endl;
            return;
        }
        gc_pin<node> pin(root);
        std::cout << offset << & root << " (" << pin.get() << ") [" << get_object_mark(pin.get()) << "]" << std::endl;
        auto new_offset = offset + "    ";
        print_tree(root->m_left, new_offset);
        print_tree(root->m_right, new_offset);
    #endif
}

void print_tree(node* root, const std::string& offset = "")
{
    #ifdef DEBUG_PRINT_TREE
        std::cout << offset << "nullptr" << " (" << root << ") [" << get_object_mark(root) << "]" << std::endl;
        auto new_offset = offset + "    ";
        print_tree(root->m_left, new_offset);
        print_tree(root->m_right, new_offset);
    #endif
}

void generate_random_child(gc_ptr<node> ptr)
{
    while (true) {
        gc_ptr<node>& new_ptr = rand() % 2 ? ptr->m_left : ptr->m_right;
        if (!new_ptr || rand() % 4 == 0) {
            new_ptr = create_gc_node();
            return;
        } else {
            ptr = new_ptr;
        }
    }

}

std::atomic<int> thread_num(0);
std::atomic<bool> gc_finished(false);

static const int TREE_DEPTH = 2;
static const int THREADS_COUNT = 4; // must be power of 2 and <= 2^(TREE_DEPTH)
const size_t LIVE_LEVEL = log_2(THREADS_COUNT);

static barrier threads_ready(THREADS_COUNT + 1);

static void* thread_routine(void* arg)
{
    gc_ptr<node>& root = * ((gc_ptr<node>*) arg);
    int num = thread_num++;
    // assign to each thread a leaf in the tree
    gc_ptr<node> ptr = root;
    for (int i = 0; i < log_2(THREADS_COUNT); ++i, num /= 2) {
        ptr = num % 2 ? ptr->m_left : ptr->m_right;
    }
    threads_ready.wait();
    while (!gc_finished) {
        generate_random_child(ptr);
//        sleep(1);
    }
}

/**
 * The following test fixture creates tree and starts several threads.
 * In each thread gc pointer to some node in the tree is saved.
 * Then the original root is saved in a raw pointer and gc_ptr to it is reset.
 * It is assumed that then test code will perform gc or one of it phases (marking, compacting),
 * and the top levels of tree, that are not reachable from thread's gc pointers, will be freed.
 *
 *          0 <--------------- root
 *         / \
 *        0   0
 *       / \  /\
 *      *  * *  *  <---------- pointers to these nodes are saved in thread's program stacks
 *     / \  /    \
 *    *  * *      * <--------- all nodes below are randomly generated in threads
 */

struct gc_test: public ::testing::Test
{
    gc_test()
    {
        srand(time(nullptr));

        gc_finished = false;
        thread_num = 0;

        std::cout << "Creating tree" << std::endl;
        root = create_tree(TREE_DEPTH);
        unmark_tree(root);
        print_tree(root);

        for (auto& thread: threads) {
            int res = thread_create(&thread, nullptr, thread_routine, (void*) &root);
            assert(res == 0);
        }
        threads_ready.wait();

        // save root in raw pointer and then null gc_ptr to collect it during gc
        gc_pin<node> pin(root);
        root_raw = pin.get();
        root.reset();
    }

    ~gc_test()
    {
        for (auto& thread: threads) {
            void* ret = nullptr;
            thread_join(thread, &ret);
        }
    }

    gc_ptr<node> root;
    pthread_t threads[THREADS_COUNT];
    node* root_raw;
};

void check_nodes(node* ptr, size_t depth)
{
    if (depth < LIVE_LEVEL) {
        EXPECT_FALSE(get_object_mark(ptr)) << "ptr=" << ptr;
    } else {
        EXPECT_TRUE(get_object_mark(ptr))  << "ptr=" << ptr;
    }
    if (ptr->m_left) {
        gc_pin<node> pin(ptr->m_left);
        check_nodes(pin.get(), depth + 1);
    }
    if (ptr->m_right) {
        gc_pin<node> pin(ptr->m_right);
        check_nodes(pin.get(), depth + 1);
    }
}

}

TEST_F(gc_test, test_marking)
{
    auto& collector = gc_garbage_collector::instance();
    std::cout << "Start marking" << std::endl;
    collector.start_marking();
    std::cout << "Wait for marking finished" << std::endl;
    collector.wait_for_marking_finished();

    gc_finished = true;

    std::cout << std::endl;
    print_tree(root_raw);
    // travers tree and check marks of objects
    check_nodes(root_raw, 0);

    collector.force_move_to_idle();
}

// This test doesn't check anything.
// Consider it is passed if nothing will crash or hang.
TEST_F(gc_test, test_gc)
{
    gc();
    gc_finished = true;
}