#include <details/mutex.h>

#include "gcmalloc.h"

#include "details/page_descriptor.h"
#include "details/heap.h"
#include "index_tree.h"

bool _GC_::is_heap_pointer(void *ptr) {
    return IT_get_page_descr(ptr) != NULL;
}

// TODO we need some gc strategy
void * _GC_::gcmalloc(size_t s, void * meta, size_t count = 1) {
    typedef precisegc::details::heap heap_type;
    heap_type& heap = heap_type::instance();
    void* ptr = heap.allocate(s * count + sizeof(Object));
    Object* obj = (Object *) (ptr + s * count);
    obj->meta = meta;
    return ptr;
}

void _GC_::set_meta_after_gcmalloc (void * ptr, void * clMeta) {
    get_object_header(ptr)->meta = clMeta;
}

Object * _GC_::get_object_header (void * ptr) {
    precisegc::details::page_descriptor* pd = (precisegc::details::page_descriptor*) IT_get_page_descr(ptr);
    size_t obj_start = (size_t) pd->get_object_start(ptr);
    Object * res = (Object *)(obj_start + pd->obj_size() - sizeof(Object));
    return res;
}

static precisegc::details::mutex get_meta_inf_mutex;

_GC_::base_meta * _GC_::get_meta_inf (void * ptr) {  /*!< get the block with meta_inf*/
    precisegc::details::mutex_lock<precisegc::details::mutex> lock(get_meta_inf_mutex);
    _GC_::base_meta * res = (_GC_::base_meta *) get_object_header(ptr);
    return res;
}

