#include "gc_garbage_collector.h"

#include <cstdint>

#include "gc_ptr.h"
#include "gc_mark.h"
#include "gc_mark_queue.h"
#include "gcmalloc.h"
#include "gc_heap.h"
#include "gc_pause.h"
#include "logging.h"

using namespace _GC_;

namespace precisegc { namespace details {

gc_garbage_collector& gc_garbage_collector::instance()
{
    static gc_garbage_collector collector;
    return collector;
}

gc_garbage_collector::gc_garbage_collector()
    : m_phase(phase::IDLE)
{}

void gc_garbage_collector::start_gc()
{
    start_marking();
}

void gc_garbage_collector::wait_for_gc_finished()
{
    wait_for_compacting_finished();
}

void gc_garbage_collector::start_marking()
{
    logging::info() << "Thread " << pthread_self() << " is requesting start of marking phase";

    lock_guard<mutex> lock(m_phase_mutex);
    assert(m_phase == phase::IDLE);
    m_phase = phase::MARKING;
    thread_create(&m_gc_thread, nullptr, start_marking_routine, nullptr); // managed thread (because of gc_pause implementation)
}

void gc_garbage_collector::wait_for_marking_finished()
{
    lock_guard<mutex> lock(m_phase_mutex);
    m_phase_cond.wait(m_phase_mutex, [this]() { return m_phase != phase::MARKING; });

    logging::info() << "Marking phase finished";
}

void gc_garbage_collector::start_compacting()
{
    logging::info() << "Thread " << pthread_self() << " is requesting start of compacting phase";

    {
        lock_guard<mutex> lock(m_phase_mutex);
        assert(m_phase == phase::COMPACTING);

        gc_pause();
        mark();
        gc_heap& heap = gc_heap::instance();
        heap.compact();
        gc_resume();

        std::atomic_thread_fence(std::memory_order_seq_cst);
        m_phase = phase::IDLE;
    }
    m_phase_cond.notify_all();
}

void gc_garbage_collector::wait_for_compacting_finished()
{
    lock_guard<mutex> lock(m_phase_mutex);
    m_phase_cond.wait(m_phase_mutex, [this]() { return m_phase == phase::IDLE; });

    logging::info() << "Compacting phase finished";
}

void* gc_garbage_collector::start_marking_routine(void*)
{
    logging::info() << "Thread " << pthread_self() << " is marking thread";

    {
        lock_guard<mutex> lock(thread_list::instance_mutex);
        thread_list& tl = thread_list::instance();
        gc_mark_queue& mark_queue = gc_mark_queue::instance();
        for (auto& handler: tl) {
            thread_handler* p_handler = &handler;
            StackMap* stack_ptr = p_handler->stack;
            if (!stack_ptr) {
                continue;
            }
            for (StackElement* root = stack_ptr->begin(); root != nullptr; root = root->next) {
                mark_queue.push(get_pointed_to(root->addr));
            }
        }
    }
    mark();

    gc_garbage_collector& gc = gc_garbage_collector::instance();
    {
        lock_guard<mutex> lock(gc.m_phase_mutex);
        gc.m_phase = phase::COMPACTING;
        gc.m_phase_cond.notify_all();
    }

    start_compacting_routine(nullptr);
}

void* gc_garbage_collector::start_compacting_routine(void* pVoid)
{
    logging::info() << "Thread " << pthread_self() << " is compacting thread";

    gc_garbage_collector& gc = gc_garbage_collector::instance();
    gc.wait_for_marking_finished();
    gc.start_compacting();
}

void gc_garbage_collector::mark()
{
    gc_mark_queue& mark_queue = gc_mark_queue::instance();
    while (!mark_queue.empty()) {
        traverse(mark_queue.pop());
    }
}

void gc_garbage_collector::traverse(void* root)
{
    if (!root || get_object_mark(root)) {
        return;
    }
    set_object_mark(root, true);

    object_meta* obj_meta = get_object_header(root);
    const class_meta* cls_meta = obj_meta->get_class_meta();
    size_t obj_size = obj_meta->get_class_meta()->get_type_size(); // sizeof array element
    auto& offsets = cls_meta->get_offsets();

    if (offsets.empty()) {
        return;
    }

    void* ptr = root;
    gc_mark_queue& mark_queue = gc_mark_queue::instance();
    for (int i = 0; i < obj_meta->get_count(); i++) {
        for (int j = 0; j < offsets.size(); j++) {
            void *p = get_pointed_to((char *) ptr + offsets[j]);
            if (p && !get_object_mark(p)) {
                mark_queue.push(p);
            }
        }
        ptr += obj_size;
    }
}

}}