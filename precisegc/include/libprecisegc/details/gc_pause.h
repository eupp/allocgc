#ifndef DIPLOMA_GC_PAUSE_H
#define DIPLOMA_GC_PAUSE_H

#include <functional>

namespace precisegc { namespace details {

void gc_pause();

void gc_resume();

void set_gc_pause_handler(const std::function<void(void)>& pause_handler);
std::function<void(void)> get_gc_pause_handler();

}}

#endif //DIPLOMA_GC_PAUSE_H
