#include "gc_init.h"

#include "details/thread_list.h"
#include "details/gc_pause.h"
//#include "details/logging.h"

namespace precisegc {

static void create_first_thread()
{
    thread_handler first_thread;
    first_thread.pthread = pthread_self();
    first_thread.stack = StackMap::getInstance();
    first_thread.flags = 0;
    first_thread.tlflags = &new_obj_flags_tl_instance;
    first_thread.routine = nullptr;
    first_thread.arg = nullptr;
    details::thread_list::instance().insert(first_thread);
}

int gc_init()
{
    create_first_thread();
    details::gc_pause_init();
    return 0;
}

}
