#include <gtest/gtest.h>

#include <cstdlib>
#include <cmath>
#include <cassert>
#include <atomic>
#include <queue>
#include <pthread.h>

#include <libprecisegc/details/utils/make_unique.hpp>
#include <libprecisegc/details/utils/scoped_thread.hpp>
#include <libprecisegc/details/utils/scope_guard.hpp>
#include <libprecisegc/details/threads/managed_thread.hpp>
#include <libprecisegc/details/collectors/serial_gc.hpp>
#include <libprecisegc/details/collectors/incremental_gc.hpp>
#include <libprecisegc/gc_ptr.h>
#include <libprecisegc/gc_new.h>
#include <libprecisegc/gc.h>
#include <libprecisegc/details/gc_mark.h>
#include <libprecisegc/details/utils/barrier.hpp>
#include <libprecisegc/details/utils/math.h>

using namespace precisegc;
using namespace precisegc::details;

#define DEBUG_PRINT_TREE

namespace {

const int TREE_DEPTH    = 2;
const int THREADS_COUNT = 4; // must be power of 2 and <= 2^(TREE_DEPTH)
const size_t LIVE_LEVEL = ::log2(THREADS_COUNT);

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
        gc_pin<node> pin = ptr.pin();
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
    gc_pin<node> pin = root.pin();
    std::cout << offset << &root << " (" << pin.get() << ") [" << get_object_mark(pin.get()) << "]" << std::endl;
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
        gc_pin<node> pin = ptr.pin();
        gc_ptr<node> new_ptr = rand() % 2 ? pin->m_left : pin->m_right;
        if (!new_ptr || rand() % 4 == 0) {
            new_ptr = create_gc_node();
            return;
        } else {
            ptr = new_ptr;
        }
    }
}

void check_nodes(node* ptr, size_t depth)
{
    if (depth < LIVE_LEVEL) {
        EXPECT_FALSE(get_object_mark(ptr)) << "ptr=" << ptr;
    } else if (depth == LIVE_LEVEL) {
        EXPECT_TRUE(get_object_mark(ptr)) << "ptr=" << ptr;
    }
    if (ptr->m_left) {
        gc_pin<node> pin = ptr->m_left.pin();
        check_nodes(pin.get(), depth + 1);
    }
    if (ptr->m_right) {
        gc_pin<node> pin = ptr->m_right.pin();
        check_nodes(pin.get(), depth + 1);
    }
}

std::atomic<int> thread_num(0);
std::atomic<bool> gc_finished(false);

utils::barrier threads_ready(THREADS_COUNT + 1);

static void* thread_routine(void* arg)
{
    gc_ptr<node>& root = *((gc_ptr<node>*) arg);
    int num = thread_num++;
    // assign to each thread a leaf in the tree
    gc_ptr<node> ptr = root;
    for (int i = 0; i < ::log2(THREADS_COUNT); ++i, num /= 2) {
        ptr = num % 2 ? ptr->m_left : ptr->m_right;
    }
    threads_ready.wait();
    while (!gc_finished) {
        generate_random_child(ptr);
//        sleep(1);
    }
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
    gc_test(std::unique_ptr<gc_strategy> new_gc)
    {
        old_gc = gc_reset_strategy(std::move(new_gc));
        auto guard = utils::make_scope_guard([this] {
            gc_set_strategy(std::move(old_gc));
        });

        srand(time(nullptr));

        gc_finished = false;
        thread_num = 0;

        std::cout << "Creating tree" << std::endl;
        root = create_tree(TREE_DEPTH);
        unmark_tree(root);
        print_tree(root);

        for (auto& thread: threads) {
            thread = threads::managed_thread::create(thread_routine, (void*) &root);
        }
        threads_ready.wait();

        // save root in raw pointer and then null gc_ptr to collect it during gc
        gc_pin<node> pin = root.pin();
        root_raw = pin.get();
        root.reset();

        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        guard.commit();
    }

    ~gc_test()
    {
        auto guard = utils::make_scope_guard([this] {
            gc_set_strategy(std::move(old_gc));
        });
        for (auto& thread: threads) {
            thread.join();
        }
    }

    gc_ptr<node> root;
    utils::scoped_thread threads[THREADS_COUNT];
    node* root_raw;
    std::unique_ptr<gc_strategy> old_gc;
};

struct serial_gc_test : public gc_test
{
    serial_gc_test()
        : gc_test(utils::make_unique<serial_gc>(gc_compacting::DISABLED, utils::make_unique<empty_policy>()))
    {
        garbage_collector = static_cast<serial_gc*>(gc_get_strategy());
    }

    serial_gc* garbage_collector;
};

struct incremental_gc_test : public gc_test
{
    incremental_gc_test()
        : gc_test(utils::make_unique<incremental_gc>(gc_compacting::DISABLED, utils::make_unique<incremental_empty_policy>()))
    {
        garbage_collector = static_cast<incremental_gc*>(gc_get_strategy());
    }

    incremental_gc* garbage_collector;
};

// This test doesn't check anything.
// Consider it is passed if nothing will crash or hang.
TEST_F(serial_gc_test, test_serial_gc)
{
    garbage_collector->gc();
    gc_finished = true;
    print_tree(root_raw);
}

TEST_F(incremental_gc_test, test_marking)
{
    incremental_gc_ops ops;

    ops.phase           = gc_phase::MARKING;
    ops.concurrent_flag = true;
    ops.threads_num     = 1;

    garbage_collector->gc_increment(ops);

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    ops.phase           = gc_phase::IDLING;
    ops.concurrent_flag = false;
    ops.threads_num     = 0;

    garbage_collector->gc_increment(ops);

    gc_finished = true;
    print_tree(root_raw);
    check_nodes(root_raw, 0);
}

TEST_F(incremental_gc_test, test_sweeping)
{
    incremental_gc_ops ops;

    ops.phase           = gc_phase::SWEEPING;
    ops.concurrent_flag = false;
    ops.threads_num     = 1;

    garbage_collector->gc_increment(ops);

    gc_finished = true;
    print_tree(root_raw);
}
