/*************************************************************************************//**
        * File: gc_new.cpp
        * Description: This file consists describing code with realiasation functions  in "gc_new.h"
        * Update: 20/10/13
*****************************************************************************************/

#include "gc_new.h"
#include "go.h"
#include <cstdio>

pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;  /* create mutex for locking function of allocating */
std::vector <void *> offsets;
//PointerList * offsets;
bool new_active = false; /* global flag. False -- out gc_new, true -- in gc_new*/
MetaInformation * list_meta_obj = new MetaInformation();
size_t counter = 0; /* stored count of occupated blocks */
int nesting_level = 0; /* gc_new nesting level */
bool no_active = false; /* flag that says gc_ptr not to add himself in roots or offsets */