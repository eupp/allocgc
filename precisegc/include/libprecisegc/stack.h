/*************************************************************************************************//**
		* File: stack.h
		* Description:	This file describes memory pool, represented as mapped continued memory area;
						Classs is realized as singleton.
*****************************************************************************************************/
#pragma once
#include <unistd.h>


#include <mutex>

#include "details/mutex.h"

/**
* @structure --- represents a one stack element;
* @field addr --- is a pointer on the approptiate gc_ptr
*/
struct StackElement {
	void * addr;
	StackElement* next;
};

class StackMap {
protected:
	typedef precisegc::details::mutex mutex_type;

public:
	StackMap();

	typedef std::unique_lock<mutex_type> lock_type;

	/// add new element
	/// @param stored pointer
	void register_stack_root (void * newAddr);

	/// delete last-added element
	void delete_stack_root (void * address);

	StackElement* begin();

	lock_type lock();

private:
	mutex_type m_mutex;
	StackElement *top;
	StackElement * free_list;
};