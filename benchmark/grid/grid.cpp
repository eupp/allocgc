#include <new>
#include <string>
#include <iostream>
#include <cassert>
#include <random>

#ifdef PRECISE_GC_SERIAL
#include "liballocgc/liballocgc.hpp"
    using namespace allocgc;
    using namespace allocgc::serial;
#endif

#ifdef PRECISE_GC_CMS
#include "liballocgc/liballocgc.hpp"
    using namespace allocgc;
    using namespace allocgc::cms;
#endif

#include "../../common/macro.hpp"
#include "../../common/timer.hpp"

#ifdef BDW_GC
#include <gc/gc.h>

    void GC_event_callback(GC_EventType event) {
        static timer tm;
        if (event == GC_EVENT_START) {
            std::cerr << "GC start time: " << tm.elapsed<std::chrono::milliseconds>() << std::endl;
        } else if (event == GC_EVENT_END) {
            std::cerr << "GC finish time: " << tm.elapsed<std::chrono::milliseconds>() << std::endl;
        }
    }
#endif

std::random_device rd;
std::default_random_engine rndeng{rd()};
std::uniform_int_distribution<int> rnd(0, std::numeric_limits<int>::max());

std::uintptr_t genrand()
{
    return (static_cast<std::uintptr_t>(rnd(rndeng)) << 32) + rnd(rndeng);
}

struct Node
{
    ptr_t(Node) top;
    ptr_t(Node) right;
    ptr_t(Node) bottom;
    ptr_t(Node) left;
    std::uintptr_t data;

    Node()
    {
        #if defined(BDW_GC)
            top = right = bottom = left = nullptr;
        #endif
        data = genrand();
    }

    Node(const Node& other)
        : top(other.top)
        , right(other.right)
        , bottom(other.bottom)
        , left(other.left)
    {
        data = genrand();
    }
};

ptr_t(Node) create_row(size_t n, ptr_t(Node) prev_row)
{
    ptr_t(Node) head = new_(Node);
    ptr_t(Node) curr = head;
    if (prev_row) {
        curr->top = prev_row;
        prev_row->bottom = curr;
        prev_row = prev_row->right;
    }
    for (size_t i = 1; i < n; ++i) {
        curr->right = new_(Node);
        curr->right->left = curr;
        if (prev_row) {
            curr->top = prev_row;
            prev_row->bottom = curr;
            prev_row = prev_row->right;
        }
        curr = curr->right;
    }
    return head;
}

ptr_t(Node) create_grid(size_t n)
{
    ptr_t(Node) grid = create_row(n, nullptr);
    ptr_t(Node) prev_row = grid;
    for (size_t i = 1; i < n; ++i) {
        prev_row = create_row(n, prev_row);
    }
    return grid;
}

ptr_t(Node) random_walk(size_t walk_n, ptr_t(Node) node)
{
    std::discrete_distribution<> d({25, 25, 25, 25});
    for (size_t i = 0; i < walk_n; ++i) {
        switch (d(rndeng)) {
            case 0: if (node->top)      node = node->top;       break;
            case 1: if (node->right)    node = node->right;     break;
            case 2: if (node->bottom)   node = node->bottom;    break;
            case 3: if (node->left)     node = node->left;      break;
        }
    }
    return node;
}

ptr_t(Node) replace_node(ptr_in(Node) node)
{
    pin_t(Node) node_pin = pin(node);
    ptr_t(Node) new_node = new_args_(Node, *node_pin);
    if (node_pin->top) {
        node_pin->top->bottom = new_node;
    }
    if (node_pin->right) {
        node_pin->right->left = new_node;
    }
    if (node_pin->bottom) {
        node_pin->bottom->top = new_node;
    }
    if (node_pin->left) {
        node_pin->left->right = new_node;
    }
    return new_node;
}

void gridtest(size_t n)
{
    const size_t iter_num = 4 * n * n;
    size_t walk_n = std::min(n, (size_t) 100);

    ptr_t(Node) grid = create_grid(n);
    ptr_t(Node) node = grid;
    for (size_t i = 0; i < iter_num; ++i) {
        node = random_walk(walk_n, node);
        ptr_t(Node) new_node = replace_node(node);
        if (node == grid) {
            grid = new_node;
        }
        node = new_node;
    }
}

int main(int argc, const char* argv[])
{
    size_t n = 0;
    bool incremental_flag = false;
    for (int i = 1; i < argc; ++i) {
        auto arg = std::string(argv[i]);
        if (arg == "--incremental") {
            incremental_flag = true;
        } else if (arg == "--size") {
            assert(i + 1 < argc);
            n = std::stod(std::string(argv[++i]));
        }
    }

#if defined(PRECISE_GC_SERIAL) || defined(PRECISE_GC_CMS)
    register_main_thread();
//        enable_logging(gc_loglevel::DEBUG);
#elif defined(BDW_GC)
    GC_INIT();
    if (incremental_flag) {
        GC_enable_incremental();
    }
//    GC_set_on_collection_event(GC_event_callback);
#endif

    gridtest(n);

    return 0;
}

