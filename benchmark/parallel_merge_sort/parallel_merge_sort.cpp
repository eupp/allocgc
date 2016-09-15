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

static const int threads_cnt        = 8; // std::thread::hardware_concurrency();
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
    List()
        : head(nullptr)
        , length(0)
    {}

    List(ptr_in(Node) head_, size_t length_)
        : head(head_)
        , length(length_)
    {}

    ptr_t(Node) head;
    size_t length;
};

ptr_t(Node) advance(ptr_t(Node) node, size_t n)
{
    while (n > 0) {
        node = node->next;
        --n;
    }
    return node;
}

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
static precisegc::details::utils::barrier tasks_ready_barrier{threads_cnt};
static precisegc::details::utils::barrier tasks_done_barrier{threads_cnt};

static std::vector<precisegc::details::utils::scoped_thread> threads{threads_cnt - 1};

List merge_sort(const List& list);

void thread_routine(const List& input, List& output)
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

        output = merge_sort(input);

        tasks_done_barrier.wait();
    }
}

void init(const std::vector<List>& inputs, std::vector<List>& outputs)
{
    srand(time(nullptr));
    for (int i = 0; i < threads_cnt - 1; ++i) {
        threads[i] = create_thread(thread_routine, std::ref(inputs[i]), std::ref(outputs[i]));
    }
}

List merge(const List& fst, const List& snd)
{
    if (fst.length == 0 && snd.length == 0) {
        return List();
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
        assert(it1 && it2);
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

    return List(res, length);
}

List merge_sort(const List& list)
{
    if (list.length == 0) {
        return List();
    } else if (list.length == 1) {
        ptr_t(Node) node = new_(Node);
        node->data = list.head->data;
        set_null(node->next);
        return List(node, 1);
    }

    size_t m = list.length / 2;
    ptr_t(Node) mid = advance(list.head, m);

    List lpart(list.head, m);
    List rpart(mid, list.length - m);

    List lsorted = merge_sort(lpart);
    List rsorted = merge_sort(rpart);

    return merge(lsorted, rsorted);
}

List parallel_merge_sort(const List& list, std::vector<List>& input, std::vector<List>& output)
{
    ptr_t(Node) it = list.head;
    for (int i = 0; i < threads_cnt - 1; ++i) {
        input[i] = List(it, nodes_per_thread);
        it = ::advance(it, nodes_per_thread);
    }

    tasks_ready_barrier.wait();

    List lst(it, nodes_per_thread);
    List sorted = merge_sort(lst);

    tasks_done_barrier.wait();

    for (int i = 0; i < threads_cnt - 1; ++i) {
        sorted = merge(output[i], sorted);
    }

    return sorted;
}

List create_list(size_t n, int mod)
{
    if (n == 0) {
        return List();
    }
    unsigned int i = 0;
    size_t length = n;
    ptr_t(Node) head = new_(Node);
    ptr_t(Node) it = head;
    head->data = rand() % mod;
    --n;
    while (n > 0) {
        it->next = new_(Node);
        it = it->next;
        it->data = i++ % mod;
        --n;
    }
    return List(head, length);
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

void routine(std::vector<List>& input, std::vector<List>& output)
{
    List list = create_list(lists_length, 16);
    List sorted = parallel_merge_sort(list, input, output);

    ptr_t(Node) it = sorted.head;
    assert(sorted.length == lists_length);
    for (size_t i = 0; i < sorted.length - 1; ++i, it = it->next) {
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
        gc_init_options ops;
//        ops.heapsize            = 16 * 4096;      // 2 Mb
        ops.threads_available   = 1;
        ops.algo                = incremental_flag ? gc_algo::INCREMENTAL : gc_algo::SERIAL;
        ops.initiation          = gc_initiation::SPACE_BASED;
        ops.compacting          = compacting_flag ? gc_compacting::ENABLED : gc_compacting::DISABLED;
        ops.loglevel            = gc_loglevel::DEBUG;
        ops.print_stat          = false;
        gc_init(ops);
    #elif defined(BDW_GC)
        GC_INIT();
        GC_allow_register_threads();
        if (incremental_flag) {
            GC_enable_incremental();
        }
    #endif

    std::vector<List> input_lists(threads_cnt - 1);
    std::vector<List> output_lists(threads_cnt - 1);

    init(input_lists, output_lists);
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
        routine(input_lists, output_lists);
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

