#include <cstdlib>
#include <atomic>
#include <thread>
#include <future>
#include <mutex>
#include <vector>
#include <cassert>
#include <iostream>
#include <functional>
#include <type_traits>
#include <utility>

#include <libprecisegc/details/utils/barrier.hpp>
#include <libprecisegc/details/utils/scope_guard.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/utils/scoped_thread.hpp>

#ifdef BDW_GC
    #define GC_THREADS
    #include <gc/gc.h>
#endif

#ifdef PRECISE_GC
    #include <libprecisegc/libprecisegc.hpp>
    #include <libprecisegc/details/threads/managed_thread.hpp>
    using namespace precisegc;
#endif

#include "../../common/macro.hpp"
#include "../../common/timer.hpp"

static const int threads_cnt        = std::thread::hardware_concurrency();
static const int nodes_per_thread   = 256;

static const int node_size    = 64;
static const int lists_count  = 1024;
static const int lists_length = threads_cnt * nodes_per_thread;

struct Node
{
    int data;
    ptr_t(Node) next;
};

struct List
{
    ptr_t(Node) head;
    size_t length;
};

template <typename Function, typename... Args>
std::thread create_thread(Function&& f, Args&&... args)
{
#ifdef PRECISE_GC
    return details::threads::managed_thread::create(std::forward<Function>(f), std::forward<Args>(args)...);
#else
    return std::thread(std::forward<Function>(f), std::forward<Args>(args)...);
#endif
};

static std::atomic<bool> done_flag{false};
static precisegc::details::utils::barrier tasks_ready_barrier{std::thread::hardware_concurrency()};
static precisegc::details::utils::barrier tasks_done_barrier{std::thread::hardware_concurrency()};

static std::vector<precisegc::details::utils::scoped_thread> threads{threads_cnt - 1};

static std::vector<List> input_lists;
static std::vector<List> output_lists;

List merge_sort(const List& list);

void thread_routine(int i)
{
    #ifdef BDW_GC
        GC_stack_base sb;
        GC_get_stack_base(&sb);
        GC_register_my_thread(&sb);
        auto guard = precisegc::details::utils::make_scope_guard([] { GC_unregister_my_thread(); });
    #endif

    while (true) {
        tasks_ready_barrier.wait();
        if (done_flag) {
            return;
        }

        output_lists[i] = merge_sort(input_lists[i]);

        tasks_done_barrier.wait();
    }
}

void init()
{
    for (int i = 0; i < threads_cnt - 1; ++i) {
        threads[i] = create_thread(thread_routine, i);
    }
    input_lists.resize(threads_cnt - 1);
    output_lists.resize(threads_cnt - 1);
}

ptr_t(Node) advance(ptr_t(Node) node, size_t n)
{
    while (n > 0) {
        node = node->next;
        --n;
    }
    return node;
}

List merge(const List& fst, const List& snd)
{
    if (fst.length == 0 && snd.length == 0) {
        return {.head = null_ptr(Node), .length = 0};
    }

    ptr_t(Node) res;
    ptr_t(Node) it1 = fst.head;
    ptr_t(Node) it2 = snd.head;
    size_t l1 = fst.length;
    size_t l2 = snd.length;
    size_t length = l1 + l2;

    if (it1->data < it2->data) {
        res = it1;
        it1 = it1->next;
        l1--;
    } else {
        res = it2;
        it2 = it2->next;
        l2--;
    }

    ptr_t(Node) dst = res;
    while (l1 > 0 && l2 > 0) {
        if (it1->data < it2->data) {
            dst->next = it1;
            dst = dst->next;
            it1 = it1->next;
            --l1;
        } else {
            dst->next = it2;
            dst = dst->next;
            it2 = it2->next;
            --l2;
        }
    }

    if (l1 > 0) {
        dst->next = it1;
    }
    if (l2 > 0) {
        dst->next = it2;
    }

    return {.head = res, .length = length};
}

List merge_sort(const List& list)
{
    if (list.length == 0) {
        return {.head = null_ptr(Node), .length = 0};
    } else if (list.length == 1) {
        ptr_t(Node) node = new_(Node);
        node->data = list.head->data;
        set_null(node->next);
        return {.head = node, .length = 1};
    }

    size_t m = list.length / 2;
    ptr_t(Node) mid = advance(list.head, m);

    List lpart = {.head = list.head, .length = m};
    List rpart = {.head = mid,       .length = list.length - m};

    List lsorted = merge_sort(lpart);
    List rsorted = merge_sort(rpart);

    return merge(lsorted, rsorted);
}

List parallel_merge_sort(const List& list)
{
    ptr_t(Node) it = list.head;
    for (int i = 0; i < threads_cnt - 1; ++i) {
        List lst = {.head = it, .length = nodes_per_thread};
        input_lists[i] = lst;
        it = ::advance(it, nodes_per_thread);
    }

    tasks_ready_barrier.wait();

    List lst = {.head = it, .length = nodes_per_thread};
    List sorted = merge_sort(lst);

    tasks_done_barrier.wait();

    for (int i = 0; i < threads_cnt - 1; ++i) {
        sorted = merge(output_lists[i], sorted);
    }

    return sorted;
}

List create_list(size_t n, int mod)
{
    if (n == 0) {
        return {.head = null_ptr(Node), .length = 0};
    }
    size_t length = n;
    ptr_t(Node) head = new_(Node);
    ptr_t(Node) it = head;
    head->data = rand() % mod;
    --n;
    while (n > 0) {
        it->next = new_(Node);
        it = it->next;
        it->data = rand() % mod;
        --n;
    }
    return {.head = head, .length = length};
}

void clear_list(List& list)
{
    #ifdef NO_GC
        ptr_t(Node) it = list.head;
        while (list.length > 0) {
            ptr_t(Node) next = it->next;
            delete_(it);
            it = next;
            --list.length;
        }
    #endif
    set_null(list.head);
    list.length = 0;
}

void routine()
{
    List list = create_list(lists_length, lists_length);
    List sorted = parallel_merge_sort(list);

    ptr_t(Node) it = sorted.head;
    assert(sorted.length == lists_length);
    for (size_t i = 0; i < sorted.length - 1; ++i) {
        assert(it->data <= it->next->data);
    }

    clear_list(list);
    clear_list(sorted);
}

int main(int argc, const char* argv[])
{
    bool incremental_flag = false;
    bool compacting_flag = false;
    for (int i = 1; i < argc; ++i) {
        auto arg = std::string(argv[i]);
        if (arg == "--incremental") {
            incremental_flag = true;
        } else if (arg == "--compacting") {
            compacting_flag = true;
        }
    }

    #if defined(PRECISE_GC)
        gc_options ops;
//        ops.heapsize            = 2 * 1024 * 1024;      // 2 Mb
        ops.threads_available   = 1;
        ops.type                = incremental_flag ? gc_type::INCREMENTAL : gc_type::SERIAL;
        ops.init                = gc_init_strategy::SPACE_BASED;
        ops.compacting          = compacting_flag ? gc_compacting::ENABLED : gc_compacting::DISABLED;
        ops.loglevel            = gc_loglevel::SILENT;
        ops.print_stat          = false;
        gc_init(ops);
    #elif defined(BDW_GC)
        GC_INIT();
        GC_allow_register_threads();
        if (incremental_flag) {
            GC_enable_incremental();
        }
    #endif

    init();
    auto guard = precisegc::details::utils::make_scope_guard([&done_flag, &tasks_ready_barrier] {
        done_flag = true;
        tasks_ready_barrier.wait();
    });

    std::cout << "Sorting " << lists_count << " lists with length " << lists_length << std::endl;
    std::cout << "Size of each list " << lists_length * sizeof(Node) << " b" << std::endl;
    std::cout << "Total memory usage " << 2 * lists_length * sizeof(Node) * lists_count << " b" << std::endl;

    timer tm;
    for (int i = 0; i < lists_count; ++i) {
        if ((i+1) % 32 == 0) {
            std::cout << (i+1) * 100 / lists_count << "%" << std::endl;
        }
        routine();
    }

    std::cout << "Completed in " << tm.elapsed<std::chrono::milliseconds>() << " ms" << std::endl;
    #if defined(BDW_GC)
        std::cout << "Completed " << GC_get_gc_no() << " collections" << std::endl;
        std::cout << "Heap size is " << GC_get_heap_size() << std::endl;
    #elif defined(PRECISE_GC)
        gc_stat stat = gc_stats();
        std::cout << "Completed " << stat.gc_count << " collections" << std::endl;
        std::cout << "Time spent in gc " << std::chrono::duration_cast<std::chrono::microseconds>(stat.gc_time).count() << " us" << std::endl;
        std::cout << "Average pause time " << std::chrono::duration_cast<std::chrono::microseconds>(stat.gc_time / stat.gc_count).count() << " us" << std::endl;
    #endif

    done_flag = true;
    tasks_ready_barrier.wait();
    guard.commit();
    for (auto& thread: threads) {
        thread.join();
    }
}

