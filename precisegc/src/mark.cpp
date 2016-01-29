#include "gcmalloc.h"

#include <cassert>

#include "details/page_descriptor.h"
#include "index_tree.h"

using namespace _GC_;

namespace _GC_
{

void set_object_pin(void* ptr, bool pinned)
{
    precisegc::details::page_descriptor* pd = (precisegc::details::page_descriptor*)IT_get_page_descr(ptr);
    pd->set_object_pin(ptr, pinned);
}

void set_object_mark(void* ptr, bool marked)
{
    precisegc::details::page_descriptor* pd = (precisegc::details::page_descriptor*)IT_get_page_descr(ptr);
    pd->set_object_mark(ptr, marked);
}

bool get_object_pin(void* ptr)
{
    precisegc::details::page_descriptor* pd = (precisegc::details::page_descriptor*)IT_get_page_descr(ptr);
    return pd->get_object_pin(ptr);
}

bool get_object_mark(void* ptr)
{
    precisegc::details::page_descriptor* pd = (precisegc::details::page_descriptor*)IT_get_page_descr(ptr);
    return pd->get_object_mark(ptr);
}


/*
mark_pin --- false => pin bit; true => mark_bit
return values:
	 0 --- successfully marked
	 1 --- already marked
	-1 --- incorrect pointer(not in the heap) */
int mark_object (void * ptr, bool mark_pin) {
	precisegc::details::page_descriptor* pd = (precisegc::details::page_descriptor*)IT_get_page_descr(ptr);
	if (mark_pin) {
        if (pd->get_object_mark(ptr)) {
            return 1;
        } else {
            pd->set_object_mark(ptr, true);
            return 0;
        }
    } else {
        if (pd->get_object_pin(ptr)) {
            return 1;
        } else {
            pd->set_object_pin(ptr, true);
            return 0;
        }
    }
}

int get_object_mark(void *ptr, bool mark_pin) {
    precisegc::details::page_descriptor* pd = (precisegc::details::page_descriptor*)IT_get_page_descr(ptr);
    if (mark_pin) {
        return pd->get_object_mark(ptr);
    } else {
        return pd->get_object_pin(ptr);
    }
}

bool mark_after_overflow() {
// TODO write it
//    myfile << "mark_after_overflow is called! ERROR! Exit!" << endl;
    assert(true);
    return true;
}

}