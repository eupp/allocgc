#include <gtest/gtest.h>

#include <cmath>
#include <vector>

#include <libprecisegc/libprecisegc.hpp>
#include <libprecisegc/details/ptrs/gc_ptr_access.hpp>
#include <libprecisegc/details/ptrs/gc_untyped_ptr.hpp>
#include <libprecisegc/details/collectors/marker.hpp>

using namespace precisegc;
using namespace precisegc::details;
using namespace precisegc::details::ptrs;

#define DEBUG_PRINT_TREE

struct node
{
    gc_ptr<node> m_left;
    gc_ptr<node> m_right;
};

struct test_root_set
{
    template <typename Functor>
    void trace(Functor&& f)
    {
        for (auto root: roots) {
            f(root);
        }
    }

    std::vector<gc_untyped_ptr*> roots;
};

struct test_pin_set
{
    template <typename Functor>
    void trace(Functor&& f)
    {
        for (auto root: pins) {
            f(root);
        }
    }

    std::vector<void*> pins;
};

gc_ptr<node> create_gc_node()
{
    return gc_new<node>();
}

gc_ptr<node> create_tree(size_t depth)
{
    std::queue<gc_ptr<node>> q;
    const size_t LEAFS_CNT = pow(2, depth - 1);
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

void mark_tree(gc_ptr<node>& ptr, size_t depth, size_t mark_depth, test_root_set& root_set)
{
    if (!ptr) {
        return;
    }
    gc_pin<node> pin = ptr.pin();
    set_object_mark(pin.get(), false);
    if (depth == mark_depth) {
        root_set.roots.push_back(&gc_ptr_access<node>::get_untyped(ptr));
    }
    mark_tree(ptr->m_left, depth + 1, mark_depth, root_set);
    mark_tree(ptr->m_right, depth + 1, mark_depth, root_set);
}

void check_nodes_marked(const gc_ptr<node>& ptr, size_t depth, size_t mark_depth, size_t max_depth)
{
    if (depth <= max_depth) {
        ASSERT_TRUE(ptr);
    } else {
        return;
    };

    gc_pin<node> pin = ptr.pin();
    if (depth < mark_depth) {
        EXPECT_FALSE(get_object_mark(pin.get())) << "ptr=" << pin.get();
    } else {
        EXPECT_TRUE(get_object_mark(pin.get())) << "ptr=" << pin.get();
    }

    check_nodes_marked(ptr->m_left, depth + 1, mark_depth, max_depth);
    check_nodes_marked(ptr->m_right, depth + 1, mark_depth, max_depth);
}

void check_nodes_pinned(const gc_ptr<node>& ptr, size_t depth, size_t pin_depth, size_t max_depth)
{
    if (depth <= max_depth) {
        ASSERT_TRUE(ptr);
    } else {
        return;
    };

    node* raw_ptr = gc_ptr_access<node>::get(ptr);
    if (depth == pin_depth) {
        EXPECT_TRUE(get_object_pin(raw_ptr)) << "ptr=" << raw_ptr;
    } else {
        EXPECT_FALSE(get_object_pin(raw_ptr)) << "ptr=" << raw_ptr;
    }

    check_nodes_pinned(ptr->m_left, depth + 1, pin_depth, max_depth);
    check_nodes_pinned(ptr->m_right, depth + 1, pin_depth, max_depth);
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

const int TREE_DEPTH    = 4;

struct marker_test: public ::testing::Test
{
    marker_test()
        : root(create_tree(TREE_DEPTH))
    {}

    gc_ptr<node> root;
    test_root_set root_set;
    test_pin_set pin_set;
    collectors::marker marker;
};

/**
 * The following test checks in case of empty root set no nodes are marked
 *
 *          0 <--------------- root
 *         / \
 *        0   0
 *       / \  /\
 *      0  0 0  0
 *     /\ /\ /\ /\
 *     00 00 00 00 <--------- all nodes are unmarked after mark()
 */
TEST_F(marker_test, test_unmarked)
{
    mark_tree(root, 1, 0, root_set);
    std::cout << "Tree before marking" << std::endl << std::endl;
    print_tree(root);

    marker.trace_roots(root_set);
    marker.mark();

    std::cout << std::endl << "Tree after marking" << std::endl << std::endl;
    print_tree(root);
    check_nodes_marked(root, 1, TREE_DEPTH + 1, TREE_DEPTH);
}

/**
 * The following test checks that all nodes accessible from roots are marked
 *
 *          0 <--------------- root
 *         / \
 *        0   0
 *       / \  /\
 *      *  * *  *  <---------- pointers to these nodes are roots
 *     /\ /\ /\ /\
 *     00 00 00 00 <--------- all nodes below should be marked by marker
 */
TEST_F(marker_test, test_roots)
{
    const size_t LIVE_LEVEL = 3;

    mark_tree(root, 1, LIVE_LEVEL, root_set);
    std::cout << "Tree before marking" << std::endl << std::endl;
    print_tree(root);

    marker.trace_roots(root_set);
    marker.mark();

    std::cout << std::endl << "Tree after marking" << std::endl << std::endl;
    print_tree(root);
    check_nodes_marked(root, 1, LIVE_LEVEL, TREE_DEPTH);
}

/**
 * The following test checks that all nodes accessible from pin set are marked and pinned nodes itself are marked as pinned
 *
 *          + <--------------- root (added to pin set)
 *         / \
 *        *   *  <--------- all nodes below should be marked by marker
 *       / \  /\
 *      *  * *  *
 *     /\ /\ /\ /\
 *     ** ** ** **
 */
TEST_F(marker_test, test_pins)
{
    mark_tree(root, 1, 0, root_set);
    std::cout << "Tree before marking" << std::endl << std::endl;
    print_tree(root);

    pin_set.pins.push_back(gc_ptr_access<node>::get(root));
    marker.trace_pins(pin_set);
    marker.mark();

    std::cout << std::endl << "Tree after marking" << std::endl << std::endl;
    print_tree(root);
    check_nodes_marked(root, 1, 1, TREE_DEPTH);
    check_nodes_pinned(root, 1, 1, TREE_DEPTH);
}