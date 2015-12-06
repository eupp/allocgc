/****************************************************************************************
		* File: go.cpp
		* Description: functions in this file realizes heap traversing (mark phase)
			and mark and sweep function
*****************************************************************************************/

#include "taginfo.h"
#include "gc_new.h"
#include "fake_roots.h"
#include "threading.h"
#ifdef DEBUGE_MODE
	size_t live_object_count = 0;
#endif

/**
* @function get_ptr
* @return pointer that is cleared from glags was setted in gc_ptr
* @param ptr --- some pointer (really is a pointer to th managed object)
*/
void * get_ptr (void * ptr) {
	if (is_composite_pointer(ptr)) {
		dprintf(" %p comp %p \n ", ptr, ((Composite_pointer *)(clear_both_flags(ptr)))->base);
//		if (!get_object_mark(clear_both_flags(ptr), true)) {
//			mark_object(clear_both_flags(ptr), true);
//		}
		return ((Composite_pointer *)(clear_both_flags(ptr)))->base;
	} else {
		dprintf(" %p !comp %p\n ", ptr, clear_stack_flag(ptr));
		return clear_stack_flag(ptr);
	}
}

void * move_ptr(void* ptr, void* value) {
	if (is_composite_pointer(ptr)) {
		((Composite_pointer*)clear_both_flags(ptr))->base = value;
		return ptr;
	} else {
		return restore_flags(value, get_both_flags(ptr));
	}
}

/**
* @function get_next_obj
* @return pointer (void *) on the object on that root or gc_ptr "v" points;
	in case v is invalide pointer to the gc_ptr "v" returns NULL.
* @param v --- pointer (like gc_ptr)
*/
void * get_next_obj(void * v) {  /* get the next object*/
	dprintf(" get_next_obj %p ", v); fflush(stdout);
	void * res = reinterpret_cast <void*> (*((size_t *)v));
	if (res == NULL)
		return NULL;

	dprintf(" res %p\n ", res); fflush(stdout);
	return clear_both_flags(res) == NULL ? NULL : get_ptr(res);
}

/**
* @function get_meta_inf
* @return object of class base_meta (object meta) that is ctored directly with managed object
* @param v --- pointer to the managed object
* TODO: it can throw exception in "v"'s incorrect pointer case
*/
//base_meta * get_meta_inf (void * v) {  /*!< get the block with meta_inf*/
//	return (base_meta*)v - 1;
//}

//void * to_get_meta_inf (void * v) {  /*!< get the block with meta_inf*/
//	return reinterpret_cast <void *> ((base_meta*)(v) + 1);
//}

/**	ifs stack throws up to max_stack_size
*	then overflow case will be
*/
#define max_stack_size 100000
void* stack[max_stack_size];
size_t stack_size;
int push (void * new_element) {
	if (stack_size >= max_stack_size) {
		return 1;
	}
	stack[stack_size++] = new_element;
	return 0;
}

void * pop () {
	if (stack_size == 0) {
		return NULL;
	}
	return stack[--stack_size];
}

/**
* @function go
* @detailed recursive traversing function (implements mark gc phase);
	marks object as alive and following its object meta,
	starting from pointer "v", it walk around reached objects graph
	by calling itself (function go) for all objects object "v" points to;
* @return nothing
* @param v --- is a current traversing object (in first call --- roots and fake roots)
*/
int cccc= 0;
int go (void * pointer, bool pin_root) {
	dprintf("go %p\n", pointer);

	if (!pointer || !is_heap_pointer(pointer)) {
		dprintf("go: return! --- NULL or NON-heap pointer %p\n", pointer);
		return 0;
	}
	if (pin_root && !get_object_mark(pointer, false)) {
		mark_object(get_meta_inf(pointer), false);
	}

	stack_size = 1;
	stack[0] = pointer;
	bool stack_overflow = false;
	while (stack_size != 0) {
		cccc++;
		void *v = pop();
		if (v == NULL || !is_heap_pointer(v)) {
			dprintf(" %p is not a heap pointer\n ", v);
			continue;
		}
		if (get_object_mark(v, true) == 0) {
			dprintf("go: alive: bm:%p v:%p\n", bm, v);
			mark_object(v, true);
			assert(get_object_mark(v, true) == 1);
		#ifdef DEBUGE_MODE
			live_object_count++;
		#endif
		} else {
			dprintf("gc: bm = %p ptr = %p already marked\n ", bm, v);
			continue;
		}
		if (is_stack_pointer(v)) {
			continue;
		}
		base_meta *bm = get_meta_inf(v);
		size_t size = bm->shell[0]; // sizeof array element
		size_t count = bm->shell[1]; // offsets count in meta information
		if (count == 0) {
			continue;
		}
		for (int i = 0; i < bm->count; i++) {
			for (int j = 0; j < count; j++) {
				void *p = get_next_obj((char *) v + bm->shell[2 + j]);
				if (p && (get_object_mark(p, true) == 0)) {
					if (push(p)) {
						// TODO: Check stack overflow case
						stack_overflow = true;
						exit(1);
					}
				}
			}
			v += size;
		}
	}
	return stack_overflow;
}

void clean_deref_roots();

/**
* @function gc
* @detailed force garbage collection call for malloc's from gcmalloc
* @return 0 in normal case; 1 in unsafe point case (nesting_level != 0)
*/
int gc (bool full) {
// TODO
//mark_and_sweep();
	long long start = nanotime();
	dprintf("gc: mark_and_sweep\n");
	pthread_mutex_lock(&gc_mutex);
		thread_handler* handler = get_thread_handler();
		if (handler->tlflags->nesting_level != 0) {
			pthread_mutex_unlock(&gc_mutex);
			return 1;
		}
		enter_safepoint(handler);
		handler->stack_top = __builtin_frame_address(0);
		if (!gc_thread) {
			gc_thread = handler;
			printf("thread %d is garbage collector\n", (int)handler->thread);
			if (full) {
				mark_and_sweep();
			} else {
				clean_deref_roots();
			}
			gc_thread = nullptr;
			exit_safepoint(handler);
			pthread_cond_broadcast(&gc_is_finished);
		} else {
			printf("Thread %d reached safepoint\n", (int)handler->thread);
			pthread_cond_signal(&safepoint_reached);
			pthread_cond_wait(&gc_is_finished, &gc_mutex);
			exit_safepoint(handler);
		}
	pthread_mutex_unlock(&gc_mutex);
	printf("gc full = %d time = %lldms\n", full, (nanotime() - start) / 1000000);
	return 0;
}

extern void* __libc_stack_end;

void mark_stack(thread_handler* thread, bool full_gc) {
// TODO
	void * stack_bottom = thread->stack_bottom;
	if (!stack_bottom) {
		stack_bottom = __libc_stack_end;
	}
	void** curr = (void**) thread->stack_top;
	assert(curr <= stack_bottom);
	while (curr <= stack_bottom) {
// TODO remove first if
if (curr == NULL) { return; }

		if (is_heap_pointer(*curr)) {
			dprintf("possible heap pointer: %p\n", *curr);
			mark_dereferenced_root(*curr, full_gc);
		}
		curr++;
	}
}

void clean_deref_roots() {
// TODO
	dprintf("Cleanup deref roots\n");
	thread_handler* handler = first_thread;
	while (handler) {
		while (!thread_in_safepoint(handler)) {
			dprintf("Waiting thread %d to reach safepoint\n", handler->thread);
			pthread_cond_wait(&safepoint_reached, &gc_mutex);
		}
		mark_stack(handler, false);
		handler = handler->next;
	}
	sweep_dereferenced_roots();
}

/**
* @function mark_and_sweep
* @detailed implements mark and sweep stop the world algorithm
*/
void mark_and_sweep() {
	dprintf("go.cpp: mark_and_sweep\n");
	dprintf("mark and sweep!\nbefore:");	//printDlMallocInfo(); fflush(stdout);
#ifdef DEBUGE_MODE
	live_object_count = 0;
	int i = 0;
	int over_count = 0;
#endif
	dprintf("roots: ");

// TODO remove the following string
//safepoint();

//	suspend_threads();
	thread_handler* handler = first_thread;
	while (handler) {
		while (!thread_in_safepoint(handler)) {
			printf("Waiting thread %d to reach safepoint\n", (int)handler->thread);
			pthread_cond_wait(&safepoint_reached, &gc_mutex);
		}
		// iterate root stack and call traversing function go
		mark_stack(handler, true);
		StackMap *stack_ptr = handler->stack;
		bool stack_overflow = false;
		for (StackElement* root = stack_ptr->begin(); root != nullptr; root = root->next) {/* walk through all roots*/
			stack_overflow |= go(get_next_obj(root->addr)); /* mark all available objects with mbit = 1*/
#ifdef DEBUGE_MODE
		i++;
	#endif
			dprintf(" root %p,", get_next_obj(*root));
		} dprintf("\n");

#ifdef DEBUGE_MODE
	printf("root count = %i; live_object_count = %zu\n", i, live_object_count);
#endif
		while (stack_overflow) {
			dprintf("mark_and_sweep: mark_after_overflow\n");
			stack_overflow = mark_after_overflow();
#ifdef DEBUGE_MODE
		over_count++;
	#endif
		}
#ifdef DEBUGE_MODE
	printf("over_count = %i\n", over_count);
#endif
		handler = handler->next;
	}
//	mark_fake_roots();
	// call sweep function (look at msmalloc)
	dprintf("call sweep\n");
//	sweep();
//	two_fingers_compact();
//	two_fingers_compact_full();
//	sweep_dereferenced_roots();
	dprintf("after: "); //printDlMallocInfo(); fflush(stdout);
}

extern size_t fixed_count;
void fix_roots() {
	thread_handler *handler = first_thread;
	while (handler) {
		StackMap *stack_ptr = handler->stack;
		for (StackElement* root = stack_ptr->begin(); root != NULL; root = root->next) {
			printf("fix_root: from %p\n", get_next_obj(root->addr));
//			fix_one_ptr(reinterpret_cast <void*> (*((size_t *)(root->addr))));
//			void * new_place = get_new_destination(get_next_obj(root->addr));
			void* new_place = nullptr;
			if (new_place) {
				*(void * *)root->addr = set_stack_flag(new_place);
				fixed_count++;
			}
			printf("\t: to %p\n", get_next_obj(root->addr));
		}
		handler = handler->next;
	}
}