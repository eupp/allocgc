#include <cstdlib>
#include <atomic>
#include <thread>
#include <future>
#include <mutex>
#include <iostream>
#include <functional>
#include <type_traits>
#include <utility>

#include "../../common/macro.h"

#include <libprecisegc/details/utils/scope_guard.hpp>

#ifdef BDW_GC
    #define GC_THREADS
    #include <gc/gc.h>
#endif

#ifdef PRECISE_GC
    #include <libprecisegc/libprecisegc.h>
    #include <libprecisegc/details/threads/managed_thread.hpp>
    using namespace precisegc;
#endif

// peak heap size - about 16 Mb
static const int node_size    = 64;
static const int lists_count  = 512;
static const int lists_length = 512;

static std::mutex threads_mutex;
static std::condition_variable threads_cond;
static size_t threads_count = 0;

template <typename Function, typename... Args>
std::thread create_thread(Function&& f, Args&&... args)
{
    #ifdef PRECISE_GC
        return details::threads::managed_thread::create(std::forward<Function>(f), std::forward<Args>(args)...);
    #else
        return std::thread(std::forward<Function>(f), std::forward<Args>(args)...);
    #endif
};

template <typename F, typename... Args>
std::future<typename std::result_of<F(Args...)>::type> launch_task(F&& f, Args&&... args)
{
    typedef typename std::result_of<F(Args...)>::type result_type;

    std::function<result_type(Args...)> functor(f);
    std::packaged_task<result_type(bool, Args...)> task([functor] (bool parallel_flag, Args&&... task_args) {
        auto guard = details::utils::make_scope_guard([parallel_flag, &threads_mutex, &threads_cond, &threads_count] {
            if (parallel_flag) {
                std::lock_guard<std::mutex> lock(threads_mutex);
                assert(threads_count != 0);
                --threads_count;
                threads_cond.notify_one();
            }
        });
        return functor(std::forward<Args>(task_args)...);
    });
    auto future = task.get_future();

    std::unique_lock<std::mutex> lock(threads_mutex);
    if (threads_count < std::thread::hardware_concurrency()) {
        ++threads_count;
        std::thread thread = create_thread(std::move(task), true, std::forward<Args>(args)...);
        thread.detach();
    } else {
        lock.unlock();
        task(false, std::forward<Args>(args)...);
    }

    return future;
};

void wait_for_tasks_complete()
{
    std::unique_lock<std::mutex> lock(threads_mutex);
    threads_cond.wait(lock, [&threads_count] { return threads_count == 0; });
}

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

    std::future<List> lsorted = launch_task(merge_sort, std::ref(lpart));
    List rsorted = merge_sort(rpart);

    return merge(lsorted.get(), rsorted);
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
    List sorted = merge_sort(list);

    ptr_t(Node) it = sorted.head;
    assert(sorted.length == lists_length);
    for (size_t i = 0; i < sorted.length - 1; ++i) {
        assert(it->data <= it->next->data);
    }

    clear_list(list);
    clear_list(sorted);
}

int main()
{
    #if defined(PRECISE_GC)
        gc_options ops;
        ops.type        = gc_type::SERIAL;
        ops.compacting  = gc_compacting::DISABLED;
        ops.loglevel    = gc_loglevel::OFF;
        ops.print_stat  = true;
        gc_init(ops);
    #elif defined(BDW_GC)
        GC_INIT();
        GC_enable_incremental();
    #endif

    for (int i = 0; i < lists_count; ++i) {
        if ((i+1) % 64 == 0) {
            std::cout << "Sorting " << i+1 << " list" << std::endl;
        }
        launch_task(routine);
    }
    wait_for_tasks_complete();
}

