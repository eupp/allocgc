#pragma once
#ifndef _DIPLOMA_MALLOC_H_
#define _DIPLOMA_MALLOC_H_


#include <stddef.h>
/**
* @class base_meta
* @brief object meta base class
* @detailed realizes base object meta class (like interface for each object meta)
*/
class base_meta {
public:
    size_t *shell;	/**< pointer on the box(meta info struct for storing offsets) of object */
    size_t count;	/**< array count */
//    virtual void del_ptr () = 0;	/**< delete meta-ptr */
//    virtual void* get_begin () = 0;	/**< get begin of object (pointer on meta)*/
};

	int set_meta_after_gcmalloc (void *, void *);
	bool is_heap_pointer        (void *);
	base_meta * get_meta_inf    (void *);

	void init_segregated_storage    (void);
	void * gcmalloc                 (size_t, void *, size_t);
	void remove_object              (void * ptr);

	int mark_object             (void * ptr, bool mark_pin);
	int clear_object_bit        (void * ptr, bool mark_pin);
	int clear_all_object_bits   (void * ptr);
	int get_object_mark         (void * ptr, bool mark_pin);

	void fix_one_ptr                (void *);
	void two_fingers_compact_full   (void);
	void two_fingers_compact        (void);
	void sweep                      (void);
    bool mark_after_overflow        (void);
#endif //_DIPLOMA_MALLOC_H_
