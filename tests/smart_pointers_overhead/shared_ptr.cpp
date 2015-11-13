#include <memory>
#include <iostream>

class tree_structure {
	std::shared_ptr<tree_structure> left;
	std::shared_ptr<tree_structure> right;
	int value;

public:
	tree_structure(int v, std::shared_ptr<tree_structure> l, std::shared_ptr<tree_structure> r) : left(l), right(r), value(v) {}

	void insert (int v) {
		if (v > value) {
			if (right) {
				right->insert(v);
			} else {
				std::shared_ptr<tree_structure> newel = new tree_structure(v, NULL, NULL);
				right = newel;
			}
		} else {
			if (left) {
				left->insert(v);
			} else {
				std::shared_ptr<tree_structure> newel = new tree_structure(v, NULL, NULL);
				left = newel;
			}
		}
	}

	void print () {
		if (left) {
			left->print();
		}
		std::cout << value << " ";
		if (right) {
			right->print();
		}
	}
};

int main (void) {
	std::shared_ptr<tree_structure> tree = new tree_structure(0, NULL, NULL);
	int n = 100;
	for (int i = 0; i < n; i++) {
		tree->insert(i);
	}

	return 0;
}

