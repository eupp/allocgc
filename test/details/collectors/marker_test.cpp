#include <gtest/gtest.h>

#include <cmath>
#include <vector>
#include <queue>

#include <liballocgc/liballocgc.hpp>
#include <liballocgc/gc_ptr.hpp>
#include <liballocgc/details/collectors/marker.hpp>

using namespace allocgc;
using namespace allocgc::serial;
using namespace allocgc::details;
using namespace allocgc::details::collectors;

#define DEBUG_PRINT_TREE

struct node
{
    gc_ptr<node> m_left;
    gc_ptr<node> m_right;
};

struct test_root_set
{
    std::vector<gc_handle*> roots;
};

struct test_pin_set
{
    std::vector<byte*> pins;
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
    allocators::memory_index::get_gc_cell((byte*) pin.get()).set_mark(false);
    if (depth == mark_depth) {
        root_set.roots.push_back(
                reinterpret_cast<gc_handle*>(&allocgc::pointers::internals::gc_ptr_access::get_untyped(ptr))
        );
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
        EXPECT_FALSE(allocators::memory_index::get_gc_cell((byte*) pin.get()).get_mark()) << "ptr=" << pin.get();
    } else {
        EXPECT_TRUE(allocators::memory_index::get_gc_cell((byte*) pin.get()).get_mark()) << "ptr=" << pin.get();
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

    node* raw_ptr = allocgc::pointers::internals::gc_ptr_access::get(ptr);
    if (depth == pin_depth) {
        EXPECT_TRUE(allocators::memory_index::get_gc_cell((byte*) raw_ptr).get_pin()) << "ptr=" << raw_ptr;
    } else {
        EXPECT_FALSE(allocators::memory_index::get_gc_cell((byte*) raw_ptr).get_pin()) << "ptr=" << raw_ptr;
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
//    byte* ptr =
    std::cout << offset << &root << " (" << pin.get() << ") [" << allocators::memory_index::get_gc_cell((byte*) pin.get()).get_mark() << "]" << std::endl;
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
        , marker(&packet_manager, nullptr)
    {}

    gc_ptr<node> root;
    test_root_set root_set;
    test_pin_set pin_set;
    collectors::packet_manager packet_manager;
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

    for (gc_handle* root: root_set.roots) {
        byte* ptr = gc_handle_access::get<std::memory_order_relaxed>(*root);
        gc_cell cell = allocators::memory_index::get_gc_cell(ptr);
        cell.set_mark(true);
        marker.add_root(cell);
    }
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

    for (gc_handle* root: root_set.roots) {
        byte* ptr = gc_handle_access::get<std::memory_order_relaxed>(*root);
        gc_cell cell = allocators::memory_index::get_gc_cell(ptr);
        cell.set_mark(true);
        marker.add_root(cell);
    }
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

    pin_set.pins.push_back((byte*) allocgc::pointers::internals::gc_ptr_access::get(root));
    for (byte* pin: pin_set.pins) {
        gc_cell cell = allocators::memory_index::get_gc_cell(pin);
        cell.set_mark(true);
        cell.set_pin(true);
        marker.add_root(cell);
    }
    marker.mark();

    std::cout << std::endl << "Tree after marking" << std::endl << std::endl;
    print_tree(root);
    check_nodes_marked(root, 1, 1, TREE_DEPTH);
    check_nodes_pinned(root, 1, 1, TREE_DEPTH);
}