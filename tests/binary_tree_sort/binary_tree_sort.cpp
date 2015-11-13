#include <cstdlib>
#include <string>
//#include <libprecisegc/libprecisegc.h>
#include "../../sources/libprecisegc.h"
#include "List.h"

// You may need to change it to fix segfault in threading.cpp:117
static const int STACK_OVERFLOW_FACTOR = 500;
static int array_size = 2;
bool debug_print = true;

void test () {
	gc_ptr<int> mas =  gc_new<int>(array_size);
	for (int i = 0; i < array_size; i++) {
		mas[i] = rand() % 30;
	}

	gc_ptr<List> list = gc_new<List, gc_ptr<int>, int>(mas, array_size);
	while (rand() % 50 != 1 && list->count() < STACK_OVERFLOW_FACTOR) {
		dprintf ("1\n"); fflush (stdout);
		list->insert(mas, array_size);
		dprintf ("2\n"); fflush (stdout);
		list->tree_sort();
		dprintf ("3\n"); fflush (stdout);
		 gc();
	}

	dprintf ("count: %i \n", list->count()); fflush (stdout);
	// mark_and_sweep();
	dprintf ("count: %i \n", list->count()); fflush (stdout);
	gc();
	list = list->tree_sort();
	gc();
}

int main (int argc, char * argv[]) {
	init_segregated_storage();
	// for (int i = 0; i < argc; i++) {
	// 	std::string arg = argv[i];
	// 	if (arg.length() == 9 && arg.compare(0, 8, "dprintf=") == 0) {
	// 		debug_print = (arg.compare(8, 1, "1") == 0);
	// 	}
	// }
//	int seed = time(0);
//	printf("seed = %d\n", seed);
//	srand(seed);
	 for (int i = 0; i < 300; i++) {
		printf("test %d\n", i);
		test();
//		gc();
	 }
	 gc();
	return 0;
}
