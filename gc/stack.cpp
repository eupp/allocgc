/*************************************************************************************************//**
		* File: stack.cpp
		* Description: includes realized primitives from stack.h
*****************************************************************************************************/
#include "stack.h"
#include <sys/mman.h>
#include <assert.h>

StackMap * StackMap::getInstance() {
	static thread_local StackMap instance;
	return &instance;
}

constexpr int PAGE_SIZE = 4096;
constexpr int ELEM_PER_PAGE = PAGE_SIZE / sizeof(StackElement);

void StackMap::register_stack_root(void * newAddr) {
	if (free_list == nullptr) {
		StackElement * data = (StackElement *) mmap(nullptr, PAGE_SIZE, PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		assert(data != MAP_FAILED);
		for (int i = 0; i < ELEM_PER_PAGE; ++i) {
			data->next = free_list;
			free_list = data;
			data++;
		}
	}
	StackElement *result = free_list;
	free_list = free_list->next;
	result->next = top;
	result->addr = newAddr;
	top = result;
}

void StackMap::delete_stack_root(void * address) {
	dprintf("void StackMap::delete_stack_root() { %p\n", top->addr);
	if (top->addr != address) {
		// if it is so, then automatic objects dectructs
		// not in the reverse order with their constructors
		// so we need to find and replace object that might be deleted
		// by object that is on the top
		dprintf("wrong element on the top ");
		StackElement * temp = top;
		while (temp != nullptr) {
			if (temp->addr == address) {
				temp->addr = top->addr;
				break;
			}
			temp = temp->next;
		}
	}
	StackElement* deleted = top;
	top = top->next;
	deleted->next = free_list;
	free_list = deleted;
}

StackElement* StackMap::begin(){
	return top;
}