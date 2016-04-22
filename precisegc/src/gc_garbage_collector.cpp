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
    , m_mark_state(marking_state::PAUSED)
    , m_gc_cycles_cnt(0)
    , m_mark_thread_launch(false)
{}

size_t gc_garbage_collector::get_gc_cycles_count() const
{
    return m_gc_cycles_cnt;
}

void gc_garbage_collector::start_marking()
{
    phase expected = phase::IDLE;
    if (m_phase.compare_exchange_strong(expected, phase::MARKING)) {
        thread_create(&m_mark_thread, nullptr, marking_routine, nullptr);
    } else {
        logging::debug() << "Thread " << pthread_self()
                         << " is requesting start of marking phase, but gc is already running: " << phase_str(m_phase);
    }
}

void gc_garbage_collector::pause_marking()
{
//    logging::info() << "ola";
    phase expected = phase::MARKING;
    if (m_phase.compare_exchange_strong(expected, phase::COMPACTING)) {
        logging::info() << "Thread " << pthread_self() << " is requesting pause of marking phase";

        void* ret;
        thread_join(m_mark_thread, &ret);
    } else {
        logging::debug() << "Thread " << pthread_self()
                         << " is requesting pause of marking phase, but marking is not running: " << phase_str(m_phase);
    }
}

void gc_garbage_collector::compact()
{
    pause_marking();

    gc_pause();
//    m_phase.store(phase::COMPACTING, std::memory_order_release);

    long long start = nanotime();

    pin_objects();
    mark();
    gc_heap& heap = gc_heap::instance();
    heap.compact();
    managed_ptr::reset_index_cache();

    printf("gc time = %lldms; heap size: %lldb\n", (nanotime() - start) / 1000000, heap.size());

    m_phase.store(phase::IDLE, std::memory_order_seq_cst);
    gc_resume();
}

void* gc_garbage_collector::marking_routine(void*)
{
    logging::info() << "Thread " << pthread_self() << " is marking thread";

    gc_garbage_collector& gc = gc_garbage_collector::instance();

    {
        gc_pause();

        logging::info() << "Copy roots";

//        gc.m_phase.store(phase::MARKING, std::memory_order_release);

        lock_guard<mutex> lock(thread_list::instance_mutex);
        thread_list& tl = thread_list::instance();
        gc.clear_queue();
        for (auto& handler: tl) {
            thread_handler* p_handler = &handler;
            auto stack = p_handler->stack;
            if (!stack) {
                continue;
            }
            for (StackElement* root = stack->begin(); root != nullptr; root = root->next) {
                gc.queue_push(get_pointed_to(root->addr));
            }
        }

        gc_resume();
    }

    static const size_t MAX_ATTEMPTS = 10;
    size_t attempts = 0;
    phase phs = gc.m_phase.load(std::memory_order_seq_cst);
    while (phs == phase::MARKING) {
        logging::info() << "Marking...";
        mark();
        ++attempts;
        phs = gc.m_phase.load(std::memory_order_seq_cst);
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
    logging::debug() << "Move to phase " << phase_str(phase::GC_OFF) << " from phase " << phase_str(m_phase);

    {
        lock_guard<mutex_type> lock(m_mark_mutex);
        m_mark_state = marking_state::OFF;
        m_mark_cond.notify_all();
    }

    void* ret;
    thread_join(m_mark_thread, &ret);
    m_mark_thread_launch = false;

    m_phase.store(phase::GC_OFF, std::memory_order_release);
}

void gc_garbage_collector::force_move_to_idle()
{
    gc_pause();
    m_phase = phase::IDLE;
    gc_resume();
}

void gc_garbage_collector::write_barrier(gc_untyped_ptr& dst_ptr, const gc_untyped_ptr& src_ptr)
{
    gc_unsafe_scope unsafe_scope;
    phase phs = m_phase.load(std::memory_order_seq_cst);
    void* p = src_ptr.get();
    dst_ptr.set(p);
    if (phs == phase::MARKING) {
        shade(p);
    }
}

void gc_garbage_collector::new_cell(managed_cell_ptr& cell_ptr)
{
//    static thread_local gc_mark_queue* queue = get_thread_handler()->mark_queue.get();
//    queue->push(cell_ptr.get());
//    cell_ptr.set_mark(true);
    gc_unsafe_scope unsafe_scope;
    phase phs = m_phase.load(std::memory_order_seq_cst);
    if (phs == phase::MARKING) {
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