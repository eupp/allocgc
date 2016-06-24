#include "List.h"
#include "Tree.h"

ListElement::ListElement (int i, gc_ptr<ListElement> nex) {
	value = i;
	next = nex;
}

gc_ptr<ListElement> ListElement::getNext (void) {
	return next;
}

void ListElement::print (void) {
	dprintf("%i ", value);
}

int ListElement::getValue (void) {
	return value;
}

List::List (gc_ptr<int> mas, int n) {
	head = gc_ptr<ListElement>();
	for (int i = 0; i < n; i++) {
		this->insert(mas[i]);
	}
}

void List::insert (int i) {
	gc_ptr <ListElement> newEl = gc_new<ListElement, int, gc_ptr<ListElement>>(i, head);
	this->head = newEl;
}

void List::insert (gc_ptr<int> mas, int n) {
	for (int i = 0; i < n; i++) {
		this->insert(mas[i]);
	}
}

void List::print () {
	gc_ptr<ListElement> temp = this->head;
	while (temp) {
		temp->print();
		temp = temp->getNext();
	}
	dprintf("\n");
}

int List::count (void) {
	gc_ptr<ListElement> temp = this->head;
	int res = 0;
	while (temp) {
		res++;
		temp = temp->getNext();
	}
	return res;
}

gc_ptr<List> List::tree_sort () {
	gc_ptr<Tree> tree = gc_new<Tree>();
	gc_ptr<ListElement> tmp = head;
	while (tmp) {
		tree->insert(tmp->getValue());
		tmp = tmp->getNext();
	}
	gc_ptr<List> res = tree->toList();
	return res;
}
