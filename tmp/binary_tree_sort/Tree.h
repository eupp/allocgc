#pragma once
#include <stdio.h>
//#include <liballocgc/liballocgc.h>
#include "../../sources/liballocgc.h"
#include "List.h"

class Node {
public:
	Node ();
	Node (int i, gc_ptr<Node> l, gc_ptr<Node> r);
	void insert (int i);
	void toList (gc_ptr<List> res);

private:
	int value;
	gc_ptr<Node> left, right;
};

class Tree {
public:
	Tree ();
	void insert (int i);
	bool isEmpty (void);
	gc_ptr<List> toList (void);

private:
	gc_ptr<Node> tree;
};