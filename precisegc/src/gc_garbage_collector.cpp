#include "gc_garbage_collector.h"

#include <cstdint>
#include <unistd.h>

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
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000ll + ts.tv_nsec;
}

inline timespec add(const timespec& a, const timespec& b)
{
    static const long BILLION = 1000000000;
    timespec res;
    res.tv_sec = a.tv_sec + b.tv_sec;
    res.tv_nsec = a.tv_nsec + b.tv_nsec;
    if (res.tv_nsec >= BILLION) {
        res.tv_nsec -= BILLION;
        res.tv_sec++;
    }
    return res;
}

inline timespec now()
{
    timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts;
}

static const long WAIT_TIMEOUT_MS = 100;

inline timespec abs_timeout()
{
    static timespec wait_timeout = {.tv_sec = 0, .tv_nsec = WAIT_TIMEOUT_MS * 1000};
    return add(now(), wait_timeout);
}

gc_garbage_collector& gc_garbage_collector::instance()
{
    static gc_garbage_collector collector;
    return collector;
}

gc_garbage_collector::gc_garbage_collector()
    : m_phase(phase::IDLE)
    , m_event(gc_event::NO_EVENT)
    , m_gc_cycles_cnt(0)
{
    int res = thread_create(&m_gc_thread, nullptr, gc_garbage_collector::gc_routine, nullptr);
    assert(res == 0);
    assert(m_phase.is_lock_free());
}

gc_garbage_collector::~gc_garbage_collector()
{
    force_move_to_no_gc();
    void* ret;
    thread_join(m_gc_thread, &ret);
}

size_t gc_garbage_collector::get_gc_cycles_count() const
{
    return m_gc_cycles_cnt;
}

void gc_garbage_collector::start_marking()
{
    gc_unsafe_scope unsafe_scope;
    phase phs = m_phase.load();

    logging::debug() << "Thread " << pthread_self()
        << " is requesting start of marking phase, current phase is: " << phase_str(m_phase);

    if (phs == phase::IDLE) {
        lock_guard<mutex_type> lock(m_event_mutex);
        if (m_event != gc_event::START_MARKING && m_event != gc_event::START_COMPACTING) {
            m_event = gc_event::START_MARKING;
            m_event_cond.notify_all();
        }
    }
}

void gc_garbage_collector::start_compacting()
{
    gc_unsafe_scope unsafe_scope;
    phase phs = m_phase.load();

    logging::debug() << "Thread " << pthread_self()
        << " is requesting start of compacting phase, current phase is: " << phase_str(m_phase);

    if (phs == phase::IDLE || phs == phase::MARKING || phs == phase::MARKING_FINISHED) {
        lock_guard<mutex_type> lock(m_event_mutex);
        if (m_event != gc_event::START_COMPACTING) {
            m_event = gc_event::START_COMPACTING;
            m_event_cond.notify_all();
        }
    }
}

void* gc_garbage_collector::gc_routine(void* pVoid)
{
    logging::info() << "Thread " << pthread_self() << " is gc thread";

    gc_garbage_collector& gc = gc_garbage_collector::instance();

    while (true) {
        gc_event event = gc_event::NO_EVENT;
        {
            lock_guard<mutex_type> lock(gc.m_event_mutex);

            timespec timeout = abs_timeout();
            gc.m_event_cond.wait(gc.m_event_mutex, [&gc] { return    gc.m_event == gc_event::START_MARKING
                                                                   || gc.m_event == gc_event::START_COMPACTING
                                                                   || gc.m_event == gc_event::MOVE_TO_IDLE
                                                                   || gc.m_event == gc_event::GC_OFF; });
            event = gc.m_event;
            gc.m_event = gc_event::NO_EVENT;
        }

        phase phs = gc.m_phase.load();

        if (event == gc_event::START_MARKING && phs == phase::IDLE) {
            long long start = nanotime();

            gc_pause();
            gc.m_phase.store(phase::MARKING);
            gc.trace_roots();
            gc_resume();

            printf("stop-the-world time = %lld mcrs;\n", (nanotime() - start) / 1000);

            mark();
            gc.m_phase.store(phase::MARKING_FINISHED);
        }
        if (event == gc_event::START_COMPACTING && (phs == phase::MARKING || phs == phase::MARKING_FINISHED)) {
            long long start = nanotime();

            gc_pause();
            gc.m_phase.store(phase::COMPACTING);
            gc.compact();
            gc.m_phase.store(phase::IDLE);
            ++gc.m_gc_cycles_cnt;
            gc_resume();

            printf("stop-the-world time = %lld mcrs;\n", (nanotime() - start) / 1000);
        }
        if (event == gc_event::START_COMPACTING && phs == phase::IDLE) {
            long long start = nanotime();

            gc_pause();
            gc.m_phase.store(phase::MARKING);
            gc.trace_roots();
            gc.mark();
            gc.m_phase.store(phase::COMPACTING);
            gc.compact();
            gc.m_phase.store(phase::IDLE);
            ++gc.m_gc_cycles_cnt;
            gc_resume();

            printf("stop-the-world time = %lld mcrs;\n", (nanotime() - start) / 1000);
        }
        if (event == gc_event::MOVE_TO_IDLE) {
            gc_pause();
            gc.m_phase.store(phase::IDLE);
            gc_resume();
        }
        if (event == gc_event::GC_OFF) {
            return nullptr;
        }
    }
}

void gc_garbage_collector::trace_roots()
{
    logging::info() << "Tracing roots...";

    lock_guard<mutex> lock(thread_list::instance_mutex);
    thread_list& tl = thread_list::instance();
    clear_queue();
    for (auto& handler: tl) {
        thread_handler* p_handler = &handler;
        auto stack = p_handler->stack;
        if (!stack) {
            continue;
        }
        for (StackElement* root = stack->begin(); root != nullptr; root = root->next) {
            queue_push(get_pointed_to(root->addr));
        }
    }
}

void gc_garbage_collector::mark()
{
    static gc_garbage_collector& gc = gc_garbage_collector::instance();
    while (!gc.queue_empty()) {
        byte* root = reinterpret_cast<byte*>(gc.queue_pop());
        if (root) {
            traverse(managed_cell_ptr(managed_ptr(root), 0));
        }
    }
}

void gc_garbage_collector::compact()
{
//    logging::info() << "Compacting...";

    long long start = nanotime();

    pin_objects();
    mark();
    gc_heap& heap = gc_heap::instance();
    heap.compact();
    managed_ptr::reset_index_cache();

//    printf("gc time = %lldmcrs; heap size: %lldb\n", (nanotime() - start) / 1000, heap.size());
}

void gc_garbage_collector::traverse(managed_cell_ptr root)
{
    static gc_garbage_collector& gc = gc_garbage_collector::instance();

//    root.lock_descriptor();
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

void gc_garbage_collector::force_move_to_no_gc()
{
//    logging::debug() << "Move to phase " << phase_str(phase::GC_OFF) << " from phase " << phase_str(m_phase);

    lock_guard<mutex_type> lock(m_event_mutex);
    m_event = gc_event::GC_OFF;
    m_event_cond.notify_all();
}

void gc_garbage_collector::force_move_to_idle()
{
    {
        lock_guard<mutex_type> lock(m_event_mutex);
        m_event = gc_event::MOVE_TO_IDLE;
        m_event_cond.notify_all();
    }
    while (m_phase.load() != phase::IDLE) {};
}

void gc_garbage_collector::write_barrier(gc_untyped_ptr& dst_ptr, const gc_untyped_ptr& src_ptr)
{
    gc_unsafe_scope unsafe_scope;
    phase phs = m_phase.load(std::memory_order_seq_cst);
    void* p = src_ptr.get();
    dst_ptr.set(p);
    if (phs == phase::MARKING) {
        bool res = shade(p);
        while (!res && phs == phase::MARKING) {
            logging::info() << "Queue overflow !!!";
            pthread_yield();
            res = shade(p);
            phs = m_phase.load(std::memory_order_seq_cst);
        }
    }
}

void gc_garbage_collector::new_cell(managed_cell_ptr& cell_ptr)
{
//    static thread_local gc_mark_queue* queue = get_thread_handler()->mark_queue.get();
//    queue->push(cell_ptr.get());
//    cell_ptr.set_mark(true);

    // do not call unsafe scope here
    // because new_cell is always called in the context of gc_new which is already marked as unsafe_scope
//    gc_unsafe_scope unsafe_scope;
    phase phs = m_phase.load(std::memory_order_seq_cst);
    if (phs == phase::MARKING || phs == phase::MARKING_FINISHED) {
//         allocate black objects
        cell_ptr.set_mark(true);
    } else {
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
        case phase::COMPACTING:
            return "Compacting";
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