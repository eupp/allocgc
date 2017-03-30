#pragma once
#include <stdio.h>
//#include <liballocgc/liballocgc.h>
#include "../../sources/liballocgc.h"

class ListElement {
public:
	ListElement () {};
	ListElement (int i, gc_ptr<ListElement> nex);
	gc_ptr<ListElement> getNext (void);
	void print (void);
	int getValue (void);

private:
	int value;
	gc_ptr<ListElement> next;
};

class List {
public:
	List () {};
	List (gc_ptr<int> mas, int n);
	void insert (int i);
	void insert (gc_ptr<int> mas, int n);
	void print ();
	int count ();
	gc_ptr<List> tree_sort ();

private:
	gc_ptr<ListElement> head;
};
