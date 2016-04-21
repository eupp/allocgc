#include "gc_garbage_collector.h"

#include <cstdint>

#include "gc_ptr.h"
#include "gc_mark.h"
#include "gc_mark_queue.h"
#include "gcmalloc.h"
#include "managed_ptr.h"
#include "gc_heap.h"
#include "gc_pause.h"
#include "logging.h"

using namespace _GC_;

namespace precisegc { namespace details {

inline long long nanotime( void ) {
    timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec * 1000000000ll + ts.tv_nsec;
}

gc_garbage_collector& gc_garbage_collector::instance()
{
    static gc_garbage_collector collector;
    return collector;
}

gc_garbage_collector::gc_garbage_collector()
    : m_phase(phase::IDLE)
    , m_gc_cycles_cnt(0)
    , m_gc_thread_launch(false)
{}

void gc_garbage_collector::start_gc()
{
    start_marking();
}

size_t gc_garbage_collector::get_gc_cycles_count() const
{
    return m_gc_cycles_cnt;
}

void gc_garbage_collector::wait_for_gc_finished()
{
    lock_guard<mutex_type> lock(m_phase_mutex);
    if (m_phase == phase::COMPACTING) {
        return;
    }
    m_phase_mutex.unlock();

    start_compacting_routine(nullptr);
    wait_for_compacting_finished();

    m_phase_mutex.lock();
    if (m_phase == phase::COMPACTING_FINISHED) {
        m_phase = phase::IDLE;
    }
    m_gc_cycles_cnt++;
    logging::info() << "Heap size after gc " << (size_t) gc_heap::instance().size() / (1024 * 1024);
}

void gc_garbage_collector::start_marking()
{
    lock_guard<mutex_type> lock(m_phase_mutex);
    if (m_phase == phase::IDLE) {
        logging::info() << "Thread " << pthread_self() << " is requesting start of marking phase";

        m_phase = phase::MARKING;
        m_phase_cond.notify_all();

        if (!m_gc_thread_launch) {
            // managed thread (because of gc_pause implementation)
            thread_create(&m_gc_thread, nullptr, start_marking_routine, nullptr);
            m_gc_thread_launch = true;
        }
    } else {
        logging::debug() << "Thread " << pthread_self()
                         << " is requesting start of marking phase, but gc is already running: " << phase_str(m_phase);
    }
}

void gc_garbage_collector::wait_for_marking_finished()
{
    lock_guard<mutex_type> lock(m_phase_mutex);
    m_phase_cond.wait(m_phase_mutex, [this]() { return m_phase == phase::MARKING_FINISHED; });

    logging::info() << "Marking phase finished";
}

void gc_garbage_collector::start_compacting()
{
    lock_guard<mutex_type> lock(m_phase_mutex);
//    m_phase_mutex.lock();

    if (m_phase == phase::IDLE || m_phase == phase::MARKING_FINISHED) {
        logging::info() << "Thread " << pthread_self() << " is requesting start of compacting phase";

        m_phase = phase::COMPACTING;

        m_phase_mutex.unlock();

        gc_pause();

        long long start = nanotime();

        pin_objects();
        mark();
        gc_heap& heap = gc_heap::instance();
        heap.compact();
        managed_ptr::reset_index_cache();
//        unpin_objects();

//        static gc_heap& heap = gc_heap::instance();
        printf("gc time = %lldms; heap size: %lldb\n", (nanotime() - start) / 1000000, heap.size());

        gc_resume();

        std::atomic_thread_fence(std::memory_order_seq_cst);

        m_phase_mutex.lock();
        assert(m_phase == phase::COMPACTING);

        m_phase = phase::COMPACTING_FINISHED;
        m_phase_cond.notify_all();
    } else {
        logging::debug() << "Thread " << pthread_self()
                         << " is requesting start of marking phase, but gc is already running: " << phase_str(m_phase);
    }
}

void gc_garbage_collector::wait_for_compacting_finished()
{
    lock_guard<mutex_type> lock(m_phase_mutex);
    m_phase_cond.wait(m_phase_mutex, [this]() { return m_phase == phase::COMPACTING_FINISHED; });

    logging::info() << "Compacting phase finished";
}

void* gc_garbage_collector::start_marking_routine(void*)
{
    logging::info() << "Thread " << pthread_self() << " is marking thread";

    gc_garbage_collector& gc = gc_garbage_collector::instance();

    while (true) {
        {
            lock_guard<mutex_type> lock(gc.m_phase_mutex);
            gc.m_phase_cond.wait(gc.m_phase_mutex, [&gc] () {
                return gc.m_phase == phase::MARKING || gc.m_phase == phase::GC_OFF;
            });
            if (gc.m_phase == phase::GC_OFF) {
                return nullptr;
            }
        }

        {
            lock_guard<mutex> lock(thread_list::instance_mutex);
            thread_list& tl = thread_list::instance();
//            gc_mark_queue& mark_queue = gc_mark_queue::instance();
            gc.clear_queue();
            for (auto& handler: tl) {
                thread_handler* p_handler = &handler;
                auto stack = p_handler->stack;
                if (!stack) {
                    continue;
                }
                StackMap::lock_type stack_lock = stack->lock();
                for (StackElement* root = stack->begin(); root != nullptr; root = root->next) {
                    gc.queue_push(get_pointed_to(root->addr));
                }
            }
        }

        mark();

        {
            lock_guard<mutex_type> lock(gc.m_phase_mutex);
            gc.m_phase = phase::MARKING_FINISHED;
            gc.m_phase_cond.notify_all();
        }
    }
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
//    gc_mark_queue& mark_queue = gc_mark_queue::instance();
    static gc_garbage_collector& gc = gc_garbage_collector::instance();
    while (!gc.queue_empty()) {
        byte* root = reinterpret_cast<byte*>(gc.queue_pop());
        if (root) {
            traverse(managed_cell_ptr(managed_ptr(root), 0));
        }
    }
}

void gc_garbage_collector::traverse(managed_cell_ptr root)
{
    static gc_garbage_collector& gc = gc_garbage_collector::instance();

    root.lock_descriptor();
    assert(root.is_live());
    if (root.get_mark()) {
        return;
    }
    root.set_mark(true);

    object_meta* obj_meta = root.get_meta();
    const class_meta* cls_meta = obj_meta->get_class_meta();
    size_t obj_size = obj_meta->get_class_meta()->get_type_size(); // sizeof array element
    auto& offsets = cls_meta->get_offsets();

    if (offsets.empty()) {
        return;
    }

    root.unlock_descriptor();
    byte* ptr = root.get_cell_begin();
//    gc_mark_queue& mark_queue = gc_mark_queue::instance();
    size_t obj_count = obj_meta->get_count();
    size_t offsets_size = offsets.size();
    for (size_t i = 0; i < obj_count; i++) {
        for (size_t j = 0; j < offsets_size; j++) {
            void *p = get_pointed_to((char *) ptr + offsets[j]);
            if (p && !get_object_mark(p)) {
                gc.queue_push(p);
            }
        }
        ptr += obj_size;
    }
}

void gc_garbage_collector::force_move_to_idle()
{
    {
        lock_guard<mutex_type> lock(m_phase_mutex);
        logging::debug() << "Move to phase " << phase_str(phase::IDLE) << " from phase " << phase_str(m_phase);
        m_phase = phase::IDLE;

    }
    m_phase_cond.notify_all();
}

void gc_garbage_collector::force_move_to_no_gc()
{
    {
        lock_guard<mutex_type> lock(m_phase_mutex);
        logging::debug() << "Move to phase " << phase_str(phase::GC_OFF) << " from phase " << phase_str(m_phase);
        m_phase = phase::GC_OFF;
    }
    m_phase_cond.notify_all();

    void* ret;
    thread_join(m_gc_thread, &ret);
}

void gc_garbage_collector::write_barrier(gc_untyped_ptr& dst_ptr, const gc_untyped_ptr& src_ptr)
{
    gc_unsafe_scope unsafe_scope;
    lock_guard<mutex_type> lock(m_phase_mutex);
    void* p = src_ptr.get();
    dst_ptr.set(p);
    if (m_phase == phase::MARKING) {
        shade(p);
    }
}

void gc_garbage_collector::new_cell(managed_cell_ptr& cell_ptr)
{
//    static thread_local gc_mark_queue* queue = get_thread_handler()->mark_queue.get();
//    queue->push(cell_ptr.get());
//    cell_ptr.set_mark(true);
    lock_guard<mutex_type> lock(m_phase_mutex);
    if (m_phase == phase::MARKING || m_phase == phase::MARKING_FINISHED) {
//         allocate black objects
        cell_ptr.set_mark(true);
    }
    else {
        cell_ptr.set_mark(false);
    }
}

const char* gc_garbage_collector::phase_str(gc_garbage_collector::phase ph)
{
    switch (ph) {
        case phase::IDLE:
            return "Idle";
        case phase::MARKING:
            return "Marking";
        case phase::MARKING_FINISHED:
            return "Marking (finished)";
        case phase::COMPACTING:
            return "Compacting";
        case phase::COMPACTING_FINISHED:
            return "Compacting (finished)";
        case phase::GC_OFF:
            return "GC off";
    }
}

void gc_garbage_collector::queue_push(void* p)
{
    m_queue.push(p);
}

void* gc_garbage_collector::queue_pop()
{
    void* p = m_queue.front();
    m_queue.pop();
    return p;
}

bool gc_garbage_collector::queue_empty()
{
    if (m_queue.empty()) {
        lock_guard<mutex> lock(thread_list::instance_mutex);
        thread_list& tl = thread_list::instance();
        for (auto& thread: tl) {
            void* p = nullptr;
            bool res = thread.mark_queue->pop(p);
            while (res) {
                m_queue.push(p);
                res = thread.mark_queue->pop(p);
            }
        }
    }
    return m_queue.empty();
}

void gc_garbage_collector::clear_queue()
{
    std::queue<void*>().swap(m_queue);
}

void gc_garbage_collector::pin_objects()
{
    lock_guard<mutex> lock(thread_list::instance_mutex);
    thread_list& tl = thread_list::instance();
    for (auto& handler: tl) {
        thread_handler* p_handler = &handler;
        auto pins = p_handler->pins;
        if (!pins) {
            continue;
        }
        StackMap::lock_type stack_lock = pins->lock();
        for (StackElement* pin = pins->begin(); pin != nullptr; pin = pin->next) {
            set_object_pin(pin->addr, true);
        }
    }
}

void gc_garbage_collector::unpin_objects()
{
    lock_guard<mutex> lock(thread_list::instance_mutex);
    thread_list& tl = thread_list::instance();
    for (auto& handler: tl) {
        thread_handler* p_handler = &handler;
        auto pins = p_handler->pins;
        if (!pins) {
            continue;
        }
        StackMap::lock_type stack_lock = pins->lock();
        for (StackElement* pin = pins->begin(); pin != nullptr; pin = pin->next) {
            set_object_pin(pin->addr, false);
        }
    }
}

}}