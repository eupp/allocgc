/*************************************************************************************//**
		* File: gc_ptr.h
		* Description: This file describe smart pointer class gc_ptr
		* Detailed: gc_ptr --- is a template library pointer primitive
			use gc_ptr insted of regular C++ pointers for managed objects.
*****************************************************************************************/

#pragma once

#include <stdint.h>

#include "stack.h"
#include "gcmalloc.h"
#include "gc_pin.h"
#include "deref_roots.h"
#include "threading.h"

#define set_stack_flag(x)		(void *)	((uintptr_t)x | (uintptr_t)1)
#define set_composite_flag(x)	(void *)	((uintptr_t)x | (uintptr_t)2)
#define set_both_flags(x)		(void *)	((uintptr_t)x | (uintptr_t)3)
#define is_stack_pointer(x)		(bool)		((uintptr_t)x & (uintptr_t)1)
#define is_composite_pointer(x)	(bool)		((uintptr_t)x & (uintptr_t)2)
#define clear_stack_flag(x)		(void *)	((uintptr_t)x & ~(uintptr_t)1)
#define clear_both_flags(x)		(void *)	((uintptr_t)x & ~(uintptr_t)3)
#define get_both_flags(x)       (uintptr_t) ((uintptr_t)x & (uintptr_t)3)
#define restore_flags(x, fl)    (void*)     ((uintptr_t)x | (uintptr_t)fl)

using namespace _GC_;

/**
* @class Composite_pointer
* @brief represents tertiary pointer level;
	uses for "creation pointers inside object" support.
*/
struct Composite_pointer {
	void * pointer;	//! pointer somewhere inside object
	void * base;	//! pointer to the comprehensive object begin
	size_t ref_count;	//! reference counter
};

/**
* @class meta
* @brief template class; realizes specific meta for eatch object;
* @detailed it creates in gc_new and it it stored directly with (right before) the
	allocated object.
*/
//template <class T>
//class meta : public base_meta {
//public:
//	/// virtual del_ptr function from base_meta realization
//	void del_ptr (void) {
//		void* ptr = to_get_meta_inf(this);
//		dprintf("in del_ptr\n");
//		if (count == 1) {
//			((T*) ptr)->~T();
//		} else {
//			for (size_t i = 0; i < count; i++)
//				((T*) ptr)[i].~T();
//		}
//	}
//
//	/// virtual get_begin function from base_meta realization
//	void* get_begin (void) {
//		dprintf("in get_begin\n");
//		return reinterpret_cast <void*> (this);
//	}
//};


/**
* @class template smart pointer class gc_ptr
* @brief the class describes library pointer primitive
* @detailed template smart pointer class gc_ptr using to represent secondary pointer level and 
* 	overrides arithmetics and other operations on them.
*/
template <typename T> 
class gc_ptr {
private:
	void * ptr; /**< pointer directly on specified managed object */
	/**
	* @function get_base_ptr
	* @param ptr is a pointer directly on the managed object
	* @return pointer to the object meta (look at base_meta and class_meta in gc_new.h)
	*/
	void * get_base_ptr (void * ptr) const {
//		safepoint();
		dprintf("get_base_ptr\n");
		if (is_composite_pointer(ptr)) {
			return ((Composite_pointer *)(clear_both_flags(ptr)))->base;
		} else {
			return clear_stack_flag(ptr);
		}
	}

	/**
	* @function ptr
	* @param ptr some pointer
	* @return some correctly aligned pointer (cleared from flags)
	*/
//	T * get_ptr (void) const {
	T * get_ptr (void) {
		assert(clear_both_flags(ptr) == NULL || is_heap_pointer(clear_both_flags(ptr)));
//		safepoint();
	    assert(clear_both_flags(ptr) == NULL || is_heap_pointer(clear_both_flags(ptr)));
		dprintf("T * get_ptr\n");
		if (is_composite_pointer(ptr)) {
			return reinterpret_cast<T *>( (((Composite_pointer *)(clear_both_flags(ptr)))->pointer));
		} else {
			return reinterpret_cast<T *>(clear_stack_flag(ptr));
		}
	}

	/**
	* @constructor
	* @detailed only for gc_new, because it is unsafe to create
		gc_ptr from some pointer outside the gc_new
		following the fact that correct meta might be before the object in memory;
		only gc_new can garantie this.
	* @param p pointer managed object
	*/
	gc_ptr (T* p) {
		assert(p != NULL);
		pthread_mutex_lock(&gc_mutex);
		thread_handler *pHandler = get_thread_handler();
		pthread_mutex_unlock(&gc_mutex);
		tlvars * new_obj_flags = pHandler->tlflags;
		StackMap * stack_ptr = pHandler->stack;
		dprintf("gc_ptr(T* p) { %p\n", this);
		ptr = (void *) p;
		if (!new_obj_flags->new_active) {
			stack_ptr->register_stack_root(this);
			ptr = set_stack_flag(ptr);
			dprintf("\tstack\n");
		} else if (is_heap_pointer(this)) {
			if (new_obj_flags->no_active) {
				dprintf("\tno_active\n");
				return;
			}
			dprintf("\theap\n");
			assert(new_obj_flags->current_pointer_to_object != 0);
			new_obj_flags->offsets.push_back(reinterpret_cast <size_t> (this) - new_obj_flags->current_pointer_to_object);
		}
	}

public:
	/**
	* @return is it automatic object (in program stack) or not (i.e. in heap)
	*/
	bool is_stack_ptr () {
		dprintf("get_stack_ptr\n");
		return is_stack_pointer(ptr);
	}

	/**
	* @constructor default constructor
	* @detailed sets ptr pointer on NULL
	*/
	gc_ptr () {
		pthread_mutex_lock(&gc_mutex);
		thread_handler *pHandler = get_thread_handler();
		pthread_mutex_unlock(&gc_mutex);
		tlvars * new_obj_flags = pHandler->tlflags;
		StackMap * stack_ptr = pHandler->stack;
		dprintf("gc_ptr() { %p\n", this);
		ptr = 0;
		if (!new_obj_flags->new_active) {
			stack_ptr->register_stack_root(this);
			ptr = set_stack_flag(ptr);
			dprintf("\tstack\n");
		} else if (is_heap_pointer(this)) {
			if (new_obj_flags->no_active) {
				dprintf("\tno_active\n");
				return;
			}
			dprintf("\theap; offsets:push_back\n");
			assert(new_obj_flags->current_pointer_to_object != 0);
			new_obj_flags->offsets.push_back(reinterpret_cast <size_t> (this) - new_obj_flags->current_pointer_to_object);
		}
		dprintf("end gc_ptr()\n");
	}

	/**
	* @constructor copy constructor
	* @param p is a gc_ptr to be copied
	*/
	gc_ptr (const gc_ptr <T> &p) {
		pthread_mutex_lock(&gc_mutex);
		thread_handler *pHandler = get_thread_handler();
		tlvars * new_obj_flags = pHandler->tlflags;
		StackMap * stack_ptr = pHandler->stack;
		pthread_mutex_unlock(&gc_mutex);
		dprintf("gc_ptr (const ...)\n");
		ptr = clear_stack_flag(p.ptr); //< also set composite flag if necessary
		if (is_composite_pointer(p.ptr)) {
			((Composite_pointer *)(clear_both_flags(p.ptr)))->ref_count++;
		}
		if (!new_obj_flags->new_active) {
			dprintf("\tstack pointer\n");
			stack_ptr->register_stack_root(this);
			ptr = set_stack_flag(ptr);
		} else if (is_heap_pointer(this)) {
			if (new_obj_flags->no_active) {
				dprintf("\tno_active\n");
				return;
			}
			dprintf("\theap pointer\n");
			assert(new_obj_flags->current_pointer_to_object != 0);
			new_obj_flags->offsets.push_back(reinterpret_cast <size_t> (this) - new_obj_flags->current_pointer_to_object);
		}
	}

	/**
	* @constructor move constructor ?!?
	* @param p is gc_ptr to be moved
	*/
	gc_ptr (gc_ptr <T> &&p) {
		ptr = clear_stack_flag(p.ptr);
	}
	
	/**
	* @destructor
	* @detailed gc_ptr dectructor;
		removes itself from root stack if neccesary;
		clear composite pointer if neccesary.
	*/
	~gc_ptr () {
		dprintf("~gc_ptr: %p; ", this);
		if (is_stack_pointer(ptr)) {
			dprintf("~gc_ptr -> delete stack root: %p\n", this);
			pthread_mutex_lock(&gc_mutex);
			StackMap * stack_ptr = get_thread_handler()->stack;
			pthread_mutex_unlock(&gc_mutex);
			stack_ptr->delete_stack_root(this);
		}
		if (is_composite_pointer(ptr)) {
			dprintf("composite ptr; ");
			Composite_pointer * cp = (Composite_pointer *) clear_both_flags(ptr);
			assert(cp->ref_count > 0);
			if (cp->ref_count-- == 0) free(cp);
		}
		dprintf("~gc_ptr: ends\n");
	}

	/* reloaded operators for gc_ptr's objects */
//	T& operator* () const					{	return * get_ptr();									}
    T& operator* ()     					{	return * get_ptr();									}
//    T* operator->() const {
    gc_pin<T> operator->()  {
		T *p = get_ptr();
		if (p) {
			return gc_pin<T>(p);
		}
	}
    operator T * ()  					{	return get_ptr();									}
//    operator T * () const 					{	return get_ptr();									}
    T * get ()  						{	return get_ptr();									}
//    T * get () const 						{	return get_ptr();									}
//	T& operator[] (size_t index) const 		{
//		assert(clear_both_flags(ptr) != NULL && is_heap_pointer(clear_both_flags(ptr)));
//		T *p = get_ptr();
//		assert(clear_both_flags(ptr) != NULL && is_heap_pointer(clear_both_flags(ptr)));
//		if (p) {
//			size_t sz = sizeof(T) * ((base_meta *)get_meta_inf(p))->count;
//			register_dereferenced_root(p, sz);
//		}
//		assert(clear_both_flags(ptr) != NULL && is_heap_pointer(clear_both_flags(ptr)));
//		return p[index];
//	}
	T& operator[] (size_t index)            {
		assert(clear_both_flags(ptr) != NULL && is_heap_pointer(clear_both_flags(ptr)));
		T *p = get_ptr();
		assert(clear_both_flags(ptr) != NULL && is_heap_pointer(clear_both_flags(ptr)));
		if (p) {
			size_t sz = sizeof(T) * ((base_meta *)get_meta_inf(p))->count;
			register_dereferenced_root(p, sz);
		}
		assert(clear_both_flags(ptr) != NULL && is_heap_pointer(clear_both_flags(ptr)));
		return p[index];
	}
	bool operator == (const gc_ptr <T> &a) 	{	return a.get_ptr() == get_ptr();					}
	bool operator != (const gc_ptr <T> &a) 	{	return a.get_ptr() != get_ptr();					}
	bool operator != (const T * a) 			{	return a != get_ptr();								}
	bool operator == (const T * a) 			{	return a == get_ptr();								}
	bool operator == (const int a) 			{	return a == reinterpret_cast<size_t> (get_ptr());	}
	bool operator != (const int a) 			{	return a != reinterpret_cast<size_t> (get_ptr());	}
    bool operator == (const long int a)		{	return a == reinterpret_cast<size_t> (get_ptr());	}
    bool operator != (const long int a)		{	return a != reinterpret_cast<size_t> (get_ptr());	}
//    operator bool () const 					{	return get_ptr() != NULL;							}
    operator bool ()  					{	return get_ptr() != NULL;							}
	gc_ptr& operator = (const gc_ptr <T> &a) {
		dprintf("gc_ptr& operator =\n");
		if (is_composite_pointer(ptr)) {
			Composite_pointer * cp = (Composite_pointer *) clear_both_flags(ptr);
			assert(cp->ref_count > 0);
			if (cp->ref_count-- == 0) free(cp);
		}

		ptr = is_stack_pointer(ptr) ? set_stack_flag(a.ptr) : clear_stack_flag(a.ptr); // auto set composite flag
		if (is_composite_pointer(a.ptr)) {
			((Composite_pointer *)(clear_both_flags(a.ptr)))->ref_count++;
		}
		return *this;
	}

	/**	\fn an equivalent of the operator = (T * a)
		\brief
			use when you want to have a pointer not to the begin of some object;
			gc_ptr<R> base --- is a pointer to the begin of the object,
				uses only in GC to mark objects;
			T * pointer --- is a pointer somewhere to the middle of the object,
				it is a pointer that points.
	*/
	template <typename R>
	void attach (T * pointer, gc_ptr<R> base) {
		dprintf("attach %p \n", (void *)base);
		if (is_composite_pointer(ptr)) {
			Composite_pointer * cp = (Composite_pointer *) clear_both_flags(ptr);
			assert(cp->ref_count > 0);
			if (cp->ref_count-- == 0) free(cp);
		}

//		Composite_pointer * cp = (Composite_pointer *) my_malloc (sizeof(Composite_pointer)); {//malloc (sizeof(Composite_pointer)); {
		Composite_pointer * cp = (Composite_pointer *) malloc (sizeof(Composite_pointer)); {
			cp->base		= (void *) base;
			cp->pointer		= (void *) pointer;
			cp->ref_count	= 1;
		}
		ptr = is_stack_pointer(ptr) ? set_both_flags(cp) : set_composite_flag(cp);
//		safepoint();
	}
	/**	\fn nullify function
		\brief nullifies current object
	*/
	void setNULL () {
		dprintf("setNULL\n");
		if (is_composite_pointer(ptr)) {
			Composite_pointer * cp = (Composite_pointer *) clear_both_flags(ptr);
			assert(cp->ref_count > 0);
			if (cp->ref_count-- == 0) free(cp);
		}
		ptr = 0;
//		safepoint();
	}

	/** look at gc_new.h */
	template <class R, typename ... Types>
	friend
	gc_ptr<R> gc_new (Types ... , size_t);

	/** look at go.h */
	inline
	friend
	void * get_next_obj (void * v);

	/**	\fn print
		\brief prints gc_ptr stamp
	*/
	void print (void) {
		printf("\ngc_ptr: %p\n", this);
		printf("\tpointer: %p\n", ptr);
		printf("\tis stack: %i\n", is_stack_pointer(ptr));
		printf("\tis composite: %i\n", is_composite_pointer(ptr));
		if (is_composite_pointer(ptr)) {
			Composite_pointer * cp = (Composite_pointer *)(clear_both_flags(ptr));
			printf("\tbase: %p pointer: %p ref_count: %zu\n", cp->base, cp->pointer, cp->ref_count);
		}
	}
};
