#include <nonius/nonius.h++>

#include <cstdlib>
#include <atomic>
#include <iostream>

#include "libprecisegc/libprecisegc.h"

using namespace std;
using namespace precisegc;

namespace {
std::atomic<size_t> threads_cnt(0);
}

struct merge_routine_data
{
    gc_ptr<int[]> m_first;
    gc_ptr<int[]> m_last;
    gc_ptr<int[]> m_dest;
};

void* merge_sort_routine(void*);

void merge(gc_ptr<int[]> first1, gc_ptr<int[]> last1, gc_ptr<int[]> first2, gc_ptr<int[]> last2, gc_ptr<int[]> dest)
{
    gc_pin<int[]> f1(first1);
    gc_pin<int[]> f2(first2);
    gc_pin<int[]> d(dest);
    size_t size1 = last1 - first1;
    size_t size2 = last2 - first2;
    size_t i = 0, j = 0, k = 0;
    while (i < size1 && j < size2) {
        d[k++] = f1[i] < f2[j] ? f1[i++] : f2[j++];
    }
    while (i < size1) {
        d[k++] = f1[i++];
    }
    while (j < size2) {
        d[k++] = f2[j++];
    }
}

void merge_sort(gc_ptr<int[]> first, gc_ptr<int[]> last, gc_ptr<int[]> dest)
{
    if (first == last) {
        return;
    }

    size_t size = last - first;
    gc_pin<int[]> f(first);
    gc_pin<int[]> d(dest);
    if (size == 1) {
        *d = *f;
        return;
    }

    size_t m = size / 2;
    gc_ptr<int[]> mid = first + m;

    pthread_t thread;
    merge_routine_data merge_data = {first, mid, dest};

    const size_t SPAWN_LB = 16;
    if (size <= SPAWN_LB) {
        merge_sort(first, mid, dest);
    } else {
        int res = thread_create(&thread, nullptr, merge_sort_routine, (void*) &merge_data);
        assert(res == 0);
        ++threads_cnt;
    }
    merge_sort(mid, last, dest + m);

    if (size > SPAWN_LB) {
        void* ret;
        thread_join(thread, &ret);
    }
    merge(dest, dest + m, dest + m, dest + size, dest);
}

void* merge_sort_routine(void* arg)
{
    merge_routine_data* data = (merge_routine_data*) arg;
    merge_sort(data->m_first, data->m_last, data->m_dest);
    return nullptr;
}

NONIUS_BENCHMARK("parallel_merge_sort", [](nonius::chronometer meter)
{
    gc_init();
    threads_cnt = 1;
    const size_t ARRAY_SIZE = 200;

    gc_ptr<int[]> first = gc_new<int[]>(ARRAY_SIZE);
    gc_ptr<int[]> last = first + ARRAY_SIZE;
    gc_ptr<int[]> dest = gc_new<int[]>(ARRAY_SIZE);

    gc_pin<int[]> p(first);
    for (size_t i = 0; i < ARRAY_SIZE; ++i) {
        p[i] = rand() % ARRAY_SIZE;
    }

    meter.measure([first, last, dest] {
        merge_sort(first, last, dest);
    });

    gc_pin<int[]> sorted(dest);
    for (size_t i = 0; i < ARRAY_SIZE - 1; ++i) {
        assert(sorted[i] <= sorted[i+1]);
//        std::cout << sorted[i] << " ";
    }
//    std::cout << std::endl << "Threads spawned: " << threads_cnt << std::endl;
});