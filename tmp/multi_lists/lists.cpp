#include "../../sources/liballocgc.h"

const int ObjArraySize = 5;
const int ThreadsCount = 4;

struct Obj {
	gc_ptr<int> mas;
	Obj () {
		mas = gc_new<int>(ObjArraySize);
		for (int i = 0; i < ObjArraySize; i++) {
			mas[i] = i;
		}
	}
};

void print_obj (gc_ptr<Obj> o) {
	for (int i = 0 ; i < ObjArraySize; i++) {
		printf("%i ", o->mas[i]);
	}
	printf("\n");
}

struct ListElement {
	int a;
	double b;
	gc_ptr<Obj> obj;
	ListElement (int a = 6, double b = 7) : a(a), b(b) {
		obj = gc_new<Obj>();
	}
};

void print_list_element (gc_ptr<ListElement> list_el) {
	printf("a = %i \n b = %d \n", list_el->a, list_el->b);
	print_obj(list_el->obj);
	printf("\n");
}

struct List {
	gc_ptr<ListElement> el;
	gc_ptr<List> next;
	List () {
		el = gc_new<ListElement>();
	}
	List (gc_ptr<List> next) : next(next) {
		el = gc_new<ListElement>();
	}
};
void print_list (gc_ptr<List> list) {
	gc_ptr<List> temp = list;
	while (temp != NULL) {
		printf("printf element:\n");
		print_list_element(temp->el);
		printf("\n");
		temp = temp->next;
	}
	printf("\n");
}

void list_insert(gc_ptr<List> list, size_t i) {
	gc_ptr<List> new_el = gc_new<List>();
	list->el->a = (int) i; list->el->b = (double) i;
	new_el->next = list->next;
	list->next = new_el;

}

void list_remove(gc_ptr<List> list, size_t i) {
	gc_ptr<List> temp = list;
	if (!temp) { return; }
	while (temp->next) {
		if (temp->next->el->b == (double) i) {
			temp->next = temp->next->next;
			return;
		}
		temp = temp->next;
	}
}

void* list_routine(void* arg) {
	gc_ptr<List> list = gc_new<List>();
	for (size_t i = 0, j = 5, k = 0; true; i++, k++) {
		if (i % j == 0) {
			list_remove(list, i / 2);
		} else {
			list_insert(list, i);
		}
		if (k == 50) {
			k = 0;
			print_list(list);
		}
		gc();
	}
}

void test (void) {
	pthread_t threads[ThreadsCount];
	for (int i = 0 ; i < ThreadsCount; i++) {
		thread_create(&threads[i], nullptr, list_routine, NULL);
	}

	for (int i = 0 ; i < ThreadsCount; i++) {
		thread_join(threads[i], nullptr);
	}
}

int main (void) {
	init_segregated_storage();
	test();

	return 0;
}
