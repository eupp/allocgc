/*************************************************************************************//**
        * File: gc_ptr.h
        * Description: This file describe smart pointer class gc_ptr
*****************************************************************************************/

#pragma once
#include <cstdio>
#include "stack.h"
#include <stdint.h>

/**
* \class template smart pointer class gc_ptr
* \brief the class describes smart pointer
* \detailed template smart pointer class gc_ptr using to represent pointers and 
* 	override arithmetics and other operations on them.
*/
template <typename T> 
class gc_ref {
private:
	T & ref; /**< pointer on specified type*/

public:
	gc_ref (T obj) {
		ref = &obj;
	}

	gc_ref (T & obj) {
		ref = obj;
	}

	gc_ref (T * obj) {
		ref = * obj;
	}

	/* reloaded operators for gc_ptrs objects*/
	// T& operator* () const					{	return * get_ptr(ptr);									}
	// T * operator-> () const 				{	return get_ptr(ptr);									}
	operator T * () const 					{	return get_ptr(ptr);									}
	T * get () const 						{	return get_ptr(ptr);									}
	T& operator[] (size_t index) const 		{	return (get_ptr(ptr))[index];							}
	T& operator[] (size_t index) 			{	return (get_ptr(ptr))[index];							}
	bool operator == (const gc_ptr <T> &a) 	{	return get_ptr(a.ptr) == get_ptr(ptr);					}
	bool operator != (const gc_ptr <T> &a) 	{	return get_ptr(a.ptr) != get_ptr(ptr);					}
	bool operator != (const T * a) 			{	return a != get_ptr(ptr);								}
	bool operator == (const T * a) 			{	return a == get_ptr(ptr);								}
	bool operator == (const int a) 			{	return a == reinterpret_cast<size_t> (get_ptr(ptr));	}
	bool operator != (const int a) 			{	return a != reinterpret_cast<size_t> (get_ptr(ptr));	}
	bool operator == (const long int a)		{	return a == reinterpret_cast<size_t> (get_ptr(ptr));	}
	bool operator != (const long int a)		{	return a != reinterpret_cast<size_t> (get_ptr(ptr));	}
	operator bool () const 					{	return get_ptr(ptr) != NULL;							}

	gc_ptr& operator = (const gc_ptr <T> &a) {
	#ifdef DEBUGE_MODE
		printf("gc_ptr& operator =\n");
	#endif
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
};
