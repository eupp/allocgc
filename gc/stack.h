/*************************************************************************************************//**
		* File: stack.h
		* Description:	This file describes memory pool, represented as mapped continued memory area;
						Classs is realized as singleton.
*****************************************************************************************************/
#pragma once
#include <unistd.h>
#include <mutex>
#include "debug_print.h"

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
	StackMap() {}
public:
	static StackMap * getInstance();

	/// add new element
	/// @param stored pointer
	void register_stack_root (void * newAddr);

	/// delete last-added element
	void delete_stack_root (void * address);

	StackElement* begin		();

private:
	StackElement *top;
	StackElement * free_list;
};