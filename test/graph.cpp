#include <gtest/gtest.h>

#include <libprecisegc/libprecisegc.hpp>

using namespace precisegc;

namespace test_gc {

struct GraphNode {
    typedef gc_ptr<GraphNode> pNode;

    static const size_t NEIGHBOR_N = 64;

    GraphNode(int data) {
        data_  = data;
        n_ = 0;
        neighbors_ = gc_new<pNode[]>(NEIGHBOR_N);
    }

    int data_;
    size_t n_;
    gc_ptr<pNode[]> neighbors_;
};

gc_ptr<int> get_data(const gc_ptr<GraphNode>& node)
{
    return take_interior<GraphNode, int, &GraphNode::data_>(node);
}

gc_ptr<GraphNode> get_neighbor(const gc_ptr<GraphNode>& node, size_t i)
{
    return node->neighbors_[i];
}

void connect(const gc_ptr<GraphNode>& x, const gc_ptr<GraphNode>& y)
{
    x->neighbors_[x->n_++] = y;
}

}

TEST(graph_test, test_gc)
{
    using namespace test_gc;

    gc_ptr<GraphNode> x = gc_new<GraphNode>(23);
    gc_ptr<GraphNode> y = gc_new<GraphNode>(42);
    connect(x, y);

    gc_ptr<GraphNode> z = get_neighbor(x, 0);
    gc_ptr<int> d = get_data(z);

    ASSERT_EQ(42, *d);
}

namespace test_raw {

struct GraphNode {
    typedef GraphNode* pNode;

    static const size_t NEIGHBOR_N = 64;

    GraphNode(int data) {
        data_  = data;
        n_ = 0;
        neighbors_ = new pNode[NEIGHBOR_N];
    }

    ~GraphNode() {
        delete[] neighbors_;
    }

    int data_;
    size_t n_;
    pNode* neighbors_;
};

int* get_data(GraphNode* node)
{
    return &(node->data_);
}

GraphNode* get_neighbor(GraphNode* node, size_t i)
{
    return node->neighbors_[i];
}

void connect(GraphNode* x, GraphNode* y)
{
    x->neighbors_[x->n_++] = y;
}

}

TEST(graph_test, test_raw)
{
    using namespace test_raw;

    GraphNode* x = new GraphNode(23);
    GraphNode* y = new GraphNode(42);
    connect(x, y);

    GraphNode* z = get_neighbor(x, 0);
    int* d = get_data(z);

    ASSERT_EQ(42, *d);

    delete x;
    delete y;
}