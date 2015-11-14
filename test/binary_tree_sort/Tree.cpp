#include "Tree.h"

Node::Node () {}

Node::Node (int i, gc_ptr<Node> l, gc_ptr<Node> r) {
	value = i; right = r; left = l;
}

void Node::insert (int i) {
	if (value > i) {
		if (!left) {
			gc_ptr<Node> l, r;
			gc_ptr<Node> newEl = gc_new<Node, int, gc_ptr<Node>, gc_ptr<Node>>(i, l, r);
			left = newEl;
		} else {
			left->insert(i);
		}
	} else {
		if (!right) {
			gc_ptr<Node> l, r;
			gc_ptr<Node> newEl = gc_new<Node, int, gc_ptr<Node>, gc_ptr<Node>>(i, l, r);
			right = newEl;
		} else {
			right->insert(i);
		}
	}
}

void Node::toList (gc_ptr<List> res) {
	if (right)
		right->toList(res);
	res->insert(value);
	if (left)
		left->toList(res);
}

Tree::Tree () {}

void Tree::insert (int i) {
	if (!tree) {
		gc_ptr<Node> l, r;
		tree = gc_new<Node, int, gc_ptr<Node>, gc_ptr<Node>>(i, l, r);
	} else {
		tree->insert (i);
	}
}

bool Tree::isEmpty (void) {
	return !tree;
}

gc_ptr<List> Tree::toList (void) {
	gc_ptr<List> res = gc_new<List> ();
	tree->toList(res);
	return res;
}
