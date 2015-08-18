#pragma once

namespace _GC_ {
	/**
	* @class base_meta
	* @brief object meta base class
	* @detailed realizes base object meta class (like interface for each object meta)
	*/
    class base_meta {
    public:
	    size_t *shell; /**< pointer on the box(meta info struct for storing offsets) of object */
	    size_t count;  /**< array count */
    };

    int set_meta_after_gcmalloc(void *, void *);
    bool is_heap_pointer(void *);
    base_meta *get_meta_inf(void *);
    void init_segregated_storage(void);

    void *gcmalloc(size_t, void *, size_t);
    void remove_object(void *ptr);

    int mark_object(void *ptr, bool mark_pin);
    int clear_object_bit(void *ptr, bool mark_pin);
    int clear_all_object_bits(void *ptr);
    int get_object_mark(void *ptr, bool mark_pin);

	void * get_new_destination (void *);

    void fix_one_ptr(void *);
    void two_fingers_compact_full(void);
//    void two_fingers_compact(void);
//    void sweep(void);
    bool mark_after_overflow(void);
}