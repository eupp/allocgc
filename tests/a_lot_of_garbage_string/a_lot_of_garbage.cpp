#include <iostream>
#include <stdio.h>
#include "GCString.h"

using std::cout;
using std::endl;

int array_size = 100;

class Node {
	gc_ptr<GCString> str;
public:
	gc_ptr<Node> left, right;

	Node() {}
	Node(gc_ptr<char> sin) { str = gc_new<GCString, const char *>(sin);}
	Node(gc_ptr<Node> l, gc_ptr<Node> r) {
		left = l; right = r;
	}
	Node(gc_ptr<Node> l, gc_ptr<Node> r, const char * sin) {
		left = l; right = r;
		str = gc_new<GCString, const char *>(sin);
	}
	Node(gc_ptr<Node> l, gc_ptr<Node> r, gc_ptr<char> sin) {
		left = l; right = r;
		str = gc_new<GCString, const char *>(sin);
	}

	void setString (gc_ptr<GCString> s) {
		str = s;
	}

	void print () {
		str->print();
	}
};
int node_count = 0;
gc_ptr<Node> makeTree (int depth, gc_ptr<char> array) {
	depth--;
	if (depth == 0) {
		node_count++;
		std::cout << "gc_new<Node>();" << node_count << std::endl;
		mark_and_sweep();
		gc_ptr<Node> res = gc_new<Node, gc_ptr<char>>(array);
		std::cout << "res: " << node_count << " " << res.is_stack_ptr()
			<< " points to " << res.get() << " this: " << &res << std::endl;
		// res = gc_new<Node, gc_ptr<char>>(array);
		std::cout << "gc_new<Node>() --- 2 m&s;" << node_count << " " << res.is_stack_ptr()
			<< " points to " << res.get() << " this: " << &res << std::endl;
		mark_and_sweep();
		return res;
	}
	else {
		node_count++;
		// mark_and_sweep();
		std::cout << "gc_new<Node> ... ;" << node_count << std::endl;
		// gc_ptr<Node> res = gc_new<Node, gc_ptr<Node>, gc_ptr<Node>, gc_ptr<char>>
			// (makeTree(depth, array), makeTree(depth, array), array);
		// res = gc_new<Node, gc_ptr<Node>, gc_ptr<Node>, gc_ptr<char>>
		// 	(makeTree(depth, array), makeTree(depth, array), array);
		// return res;
		return gc_new<Node, gc_ptr<Node>, gc_ptr<Node>, gc_ptr<char>>
			(makeTree(depth, array), makeTree(depth, array), array);
	}
}

gc_ptr<Node> createTree (int depth) {
	gc_ptr<char> mas;
	mas = gc_new<char>(array_size);
	for (int i = 0; i < array_size; i++) {
		mas[i] = i + '0';
	}
	return makeTree(depth, mas);
}

void print (gc_ptr<Node> node) {
	if (node.get() == NULL) {
		std::cerr << node.get() << " NULL\n";
		return;
	}
	std::cerr << node.get() << " not NULL\n";
	node->print();
	print(node->left);
	print(node->right);
}

void changeTree (gc_ptr<Node> node, gc_ptr<GCString> s) { //const char * str) {
	if (node.get() == NULL) return;

	// gc_ptr<GCString> s = gc_new<GCString, const char *>(str);
	node->setString(s);

	changeTree(node->left, s);
	changeTree(node->right, s);
}

void func () {
	gc_ptr<Node> node;
	node = createTree(10);
	std::cout << "func: ";
	mark_and_sweep();

	printf("change"); fflush(stdout);
	// print(node);
	printf("change"); fflush(stdout);
	gc_ptr<GCString> s = gc_new<GCString, const char *>("qwertyuiopasdfghjkl;");
	changeTree(node, s);
	// mark_and_sweep();
	// print(node);
	// mark_and_sweep();
	// print(node);
	std::cout << "func: end" << std::endl;
	mark_and_sweep();
	std::cout << "func: end" << std::endl;
}

int main (void) {
	func();
	mark_and_sweep();

	return 0;
}