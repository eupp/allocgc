#include <string>
#include <iostream>
#include <sys/time.h>
#include <cstdlib>
#include <libprecisegc/libprecisegc.h>

using std::cout;
using std::endl;
using std::string;

//<! a node element of AVL tree
struct AVLNode {
	int key;
	unsigned char height;
	gc_ptr<AVLNode> left;
	gc_ptr<AVLNode> right;
	AVLNode() : key(0), height(0) {}
	AVLNode(int k) { key = k; height = 1; }
};

//<! returns AVLNode p height
unsigned char height (gc_ptr<AVLNode> p) {
	return p ? p->height : 0;
}

//<! returns AVLNode p balance factor
int bfactor (gc_ptr<AVLNode> p) {
	return height(p->right) - height(p->left);
}

//<! recalculate AVLNode p height
void fixheight (gc_ptr<AVLNode> p) {
	unsigned char hl = height(p->left);
	unsigned char hr = height(p->right);
	p->height = (hl > hr ? hl : hr) + 1;
}

gc_ptr<AVLNode> small_left_rotate (gc_ptr<AVLNode> a) {
	gc_ptr<AVLNode> res = gc_new<AVLNode, int>(a->right->key);
	res->right = a->right->right;
	res->left = gc_new<AVLNode, int>(a->key);
	res->left->left = a->left;
	res->left->right = a->right->left;

	fixheight(res->left);
	fixheight(res);
	return res;
}

gc_ptr<AVLNode> big_left_rotate (gc_ptr<AVLNode> a) {
	gc_ptr<AVLNode> res = gc_new<AVLNode, int>(a->right->left->key);

	res->left = gc_new<AVLNode, int>(a->key);
	res->left->left = a->left;
	res->left->right = a->right->left->left;

	res->right = gc_new<AVLNode, int>(a->right->key);
	res->right->right = a->right->right;
	res->right->left = a->right->left->right;

	fixheight(res->left);
	fixheight(res->right);
	fixheight(res);
	return res;
}

gc_ptr<AVLNode> small_right_rotate (gc_ptr<AVLNode> a) {
	gc_ptr<AVLNode> res = gc_new<AVLNode, int>(a->left->key);
	res->left = a->left->left;
	res->right = gc_new<AVLNode, int>(a->key);
	res->right->right = a->right;
	res->right->left = a->left->right;

	fixheight(res->left);
	fixheight(res->right);
	fixheight(res);
	return res;
}

gc_ptr<AVLNode> big_right_rotate (gc_ptr<AVLNode> a) {
	gc_ptr<AVLNode> res = gc_new<AVLNode, int>(a->left->right->key);

	res->right = gc_new<AVLNode, int>(a->key);
	res->right->right = a->right;
	res->right->left = a->left->right->right;

	res->left = gc_new<AVLNode, int>(a->left->key);
	res->left->left = a->left->left;
	res->left->right = a->left->right->left;

	fixheight(res->left);
	fixheight(res->right);
	fixheight(res);
	return res;
}

//<! balance AVLNode p
gc_ptr<AVLNode> balance (gc_ptr<AVLNode> p) {
	fixheight(p);
	if (bfactor(p) == 2) {
		return bfactor(p->right) < 0 ? big_left_rotate(p) : small_left_rotate(p);
	}
	if (bfactor(p) == -2) {
		return bfactor(p->left) > 0 ? big_right_rotate(p) : small_right_rotate(p);
	}
	return p;
}

//<! insert new key k in AVLNode p
gc_ptr<AVLNode> insert (gc_ptr<AVLNode> p, int k) {
	if (!p) return gc_new<AVLNode, int>(k);
	if (k < p->key) {
		p->left = insert(p->left, k);
	} else {
		p->right = insert(p->right, k);
	}
	return balance(p);
}

//<! find the smallest key in tree p
gc_ptr<AVLNode> findmin (gc_ptr<AVLNode> p) {
	return p->left ? findmin(p->left) : p;
}

//<! delete the smallest element from tree p
gc_ptr<AVLNode> removemin (gc_ptr<AVLNode> p) {
	if (p->left == 0) return p->right;
	p->left = removemin(p->left);
	return balance(p);
}

//<! delete key k drom tree p
gc_ptr<AVLNode> remove (gc_ptr<AVLNode> p, int k) {
	if (!p) {
		gc_ptr<AVLNode> temp;
		return temp;
	}
	if (k < p->key) {
		p->left = remove(p->left, k);
	} else if (k > p->key) {
		p->right = remove(p->right, k);	
	} else {
		gc_ptr<AVLNode> q = p->left;
		gc_ptr<AVLNode> r = p->right;
		// delete p;
		if( !r ) return q;
		gc_ptr<AVLNode> min = findmin(r);
		min->right = removemin(r);
		min->left = q;
		return balance(min);
	}
	return balance(p);
}

//<! number of element in tree n
int elNumber(gc_ptr<AVLNode> n) {
	if (n) {
		return 1 + elNumber(n->left) + elNumber(n->right);
	}
	return 0;
}

//<! print AVL tree
void print(gc_ptr<AVLNode> n, string indent = "", bool last = true) {
	cout << indent;
	if (last) {
		cout << "\\-";
		indent += "  ";
	} else {
		cout << "|-";
		indent += "| ";
	}
	cout << n->key << "\n";

	int child_count = 2;
	if (!n->left)  child_count--;
	if (!n->right) child_count--;
	if (n->left)  print(n->left, indent, 0 == child_count - 1);
	if (n->right) print(n->right, indent, 1 == child_count - 1);
}

//<! test function
void func (void) {
	gc_ptr<AVLNode> nod = gc_new<AVLNode, int>(0);
	int size = 10;

	// insert size elements
	for (int i = 1; i < size; i++) {
		nod = insert(nod, i);
	}
	print(nod);
	mark_and_sweep();
	print(nod);

	// insert size elements
	for (int i = 1; i < size; i++) {
		nod = insert(nod, i * 10 + i);
	}
	// insert size elements
	for (int i = 1; i < size; i++) {
		nod = insert(nod, i * 13 - 7);
	}
	print(nod);
	mark_and_sweep();
	print(nod);
}

int main (void) {
	func();
	mark_and_sweep();
	gc();

	return 0;
}