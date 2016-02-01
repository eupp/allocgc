#include <details/mutex.h>

#include "gcmalloc.h"

#include "details/page_descriptor.h"
#include "gc_heap.h"
#include "index_tree.h"

bool _GC_::is_heap_pointer(void *ptr) {
    return IT_get_page_descr(ptr) != NULL;
}

// TODO we need some gc strategy
void * _GC_::gcmalloc(size_t s, const precisegc::details::class_meta* cls_meta, size_t count = 1) {
    using namespace precisegc::details;
    gc_heap& heap = gc_heap::instance();
    object_meta* obj = heap.allocate(s, count, cls_meta);
    return obj->get_object_ptr();
}

void _GC_::set_meta_after_gcmalloc(void *ptr, const precisegc::details::class_meta* cls_meta) {
    using namespace precisegc::details;
    get_object_header(ptr)->set_class_meta(cls_meta);
}

precisegc::details::object_meta* _GC_::get_object_header(void *ptr) {
    using namespace precisegc::details;
    page_descriptor* pd = (page_descriptor*) IT_get_page_descr(ptr);
    void* obj_start = pd->get_object_start(ptr);
    return object_meta::get_meta(obj_start, pd->obj_size());
}

static precisegc::details::mutex get_meta_inf_mutex;

_GC_::base_meta * _GC_::get_meta_inf (void * ptr) {  /*!< get the block with meta_inf*/
    precisegc::details::mutex_lock<precisegc::details::mutex> lock(get_meta_inf_mutex);
    _GC_::base_meta * res = (_GC_::base_meta *) get_object_header(ptr);
    return res;
}
