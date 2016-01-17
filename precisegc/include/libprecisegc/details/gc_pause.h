#ifndef DIPLOMA_GC_PAUSE_H
#define DIPLOMA_GC_PAUSE_H

#include <functional>

namespace precisegc { namespace details {

void gc_pause(const std::function<void(void)>& pause_handler);

void gc_resume();

}}

#endif //DIPLOMA_GC_PAUSE_H
