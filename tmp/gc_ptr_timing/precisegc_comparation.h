//
// Created by danya on 26.05.15.
//
#pragma once
#include "../../sources/libprecisegc.h"
#include <iostream>
#include <vector>
#include <algorithm>

//const int N = 100000;
//const int N = 10000;
//const int N = 100;
//const int N = 40;
const int N = 10;
//const int N = 5;

template <typename T>
class LinkedList {
public:
    LinkedList (void) : head() {}

    /* add new element to the head */
    void push_begin (T d) {
        gc_ptr<LinkedListElement> newEl = gc_new<LinkedListElement, T, gc_ptr<LinkedListElement>>(d, head);
        head = newEl;
    }

    T delete_begin (void) {
        T res = head->data;
        gc_ptr<LinkedListElement>  old = head;
        head = head->next;
        return res;
    }

    // throws exception -1 (means that list is empty or number > size_of list)
    T get_first (void) {
        if (head == NULL) throw -1;
        return head->data;
    }
    T get_num (int number) {
        gc_ptr<LinkedListElement> iter = head;
        for (int i = 0; i <= number && iter != NULL; i++, iter = iter->next) {
            if (i == number) { return iter->data; }
        }
        throw -1;
    }

	void delete_n (int n) {
		if (head == NULL) return;
		int i = 0;
		for (gc_ptr<LinkedListElement> iter1 = head->next, iter2 = head; iter1 != NULL; iter1 = iter1->next) {
			if (n == i) {
				iter2->next = iter1->next;
				i = 0;
			}
			else { i++;
				iter2 = iter2->next;
			}
		}
	}

	size_t size_of (void) {
        size_t size = 0;
        gc_ptr<LinkedListElement> iter = head;
        for (; iter != NULL; size++, iter = iter->next) {}
        return size;
    }

    bool contains (T element) {
        gc_ptr<LinkedListElement> iter = head;
        for (; iter != NULL; iter = iter->next) {
            if (iter->data == element) return true;
        }
        return false;
    }

    void print (void) {
        gc_ptr<LinkedListElement> iter = head;
        for (; iter != NULL; iter = iter->next) {
            std::cout << " " << iter->data;
        }
        std::cout << std::endl;
    }

    void clear (void) {
        head.setNULL();
    }

    void reverse (void) {
        gc_ptr<LinkedList<T>> new_list = gc_new<LinkedList<T>>();
        gc_ptr<LinkedListElement> iter = head;
        for (; iter != NULL; iter = iter->next) {
            head = iter;
            new_list->push_begin(iter->data);
        }
        head = new_list->head;
    }

    void stupid_bubble_sort (int n) {
        if (head == NULL) return;
        gc_ptr<LinkedListElement> tempHead = gc_new<LinkedListElement, T, gc_ptr<LinkedListElement>>(0, head), temp1 = tempHead, temp2 = tempHead;
        for (int i = 0; temp1->next != NULL && i < n; i++, temp1 = temp1->next) {
            int j;
            for (temp2 = temp1->next, j = i; temp2->next != NULL && j < n; j++, temp2 = temp2->next) {
                if (temp1->next->data > temp2->next->data) {
                    gc_ptr<LinkedListElement> new_el_1 = gc_new<LinkedListElement, T, gc_ptr<LinkedListElement>>(temp2->next->data, temp1->next->next),
                            new_el_2 = gc_new<LinkedListElement, T, gc_ptr<LinkedListElement>>(temp1->next->data, temp2->next->next);
                    if (temp2 == temp1->next) { temp2 = new_el_1; }
                    temp1->next = new_el_1; temp2->next = new_el_2;
                }
            }
        }
        head = tempHead->next;
    }


private:
    class LinkedListElement {
    public:
        LinkedListElement () {}
        LinkedListElement (T d, gc_ptr<LinkedListElement> n) : data(d), next(n) {}

        T data;
        gc_ptr<LinkedListElement> next;
    };

    gc_ptr<LinkedListElement> head;
};

template <typename T>
class Obj {
public:
    Obj () {}
    Obj (T n) : data(n) {}

    void set (T el) { data = el; }
    T get (void) { return data; }
    inline bool operator< (const Obj& o) const {
        return data < o.data;
    }
private:
    T data;
};