#define BOOST_MATH_NO_LONG_DOUBLE_MATH_FUNCTIONS
#define BOOST_MATH_PROMOTE_DOUBLE_POLICY  false

//#include <nonius/nonius.h++>
//
//#include <cstdlib>
//#include <atomic>
//#include <iostream>
//
//// Our precise GC
//#define PRECISE_GC
//
//// Boehm/Demers/Weiser conservative GC
////#define BDW_GC
//
//// std::shared_ptr (reference count)
////#define SHARED_PTR
//
//// raw pointers
////#define NO_GC
//
//#ifdef BDW_GC
//    #define GC_THREADS
//    #include <gc/gc.h>
//#endif
//
//#include "../../common/macro.h"
//
//#include "libprecisegc/libprecisegc.h"
//#include "libprecisegc/details/gc_heap.h"
//
//using namespace precisegc;
//
//using namespace std;
//
//namespace {
//std::atomic<size_t> threads_cnt(0);
//
//struct Node
//{
//    int data;
//    ptr_t(Node) next;
//};
//
//struct List
//{
//    ptr_t(Node) head;
//    size_t length;
//};
//}
//
//struct merge_routine_data
//{
//    List list;
//    ptr_t(Node) res;
//};
//
//void* merge_sort_routine(void*);
//
//ptr_t(Node) advance(ptr_t(Node) node, size_t n)
//{
//    while (n > 0) {
//        node = node->next;
//        --n;
//    }
//    return node;
//}
//
//ptr_t(Node) merge(const List& fst, const List& snd)
//{
//    if (fst.length == 0 && snd.length == 0) {
//        return null_ptr(Node);
//    }
//    ptr_t(Node) res;
//    ptr_t(Node) it1 = fst.head;
//    ptr_t(Node) it2 = snd.head;
//    size_t l1 = fst.length;
//    size_t l2 = snd.length;
//
//    if (it1->data < it2->data) {
//        res = it1;
//        it1 = it1->next;
//        l1--;
//    } else {
//        res = it2;
//        it2 = it2->next;
//        l2--;
//    }
//
//    ptr_t(Node) dst = res;
//    while (l1 > 0 && l2 > 0) {
//        if (it1->data < it2->data) {
//            dst->next = it1;
//            dst = dst->next;
//            it1 = it1->next;
//            --l1;
//        } else {
//            dst->next = it2;
//            dst = dst->next;
//            it2 = it2->next;
//            --l2;
//        }
//    }
//
//    while (l1 > 0) {
//        dst->next = it1;
//        dst = dst->next;
//        it1 = it1->next;
//        --l1;
//    }
//    while (l2 > 0) {
//        dst->next = it2;
//        dst = dst->next;
//        it2 = it2->next;
//        --l2;
//    }
//
//    return res;
//}
//
//void merge_sort(const List& list, ptr_t(Node)& res)
//{
//    if (list.length == 0) {
//        res = null_ptr(Node);
//        return;
//    } else if (list.length == 1) {
//        res = new_(Node);
//        res->data = list.head->data;
//        return;
//    }
//
//    size_t m = list.length / 2;
//    ptr_t(Node) mid = advance(list.head, m);
//
//    List lpart = {list.head, m};
//    List rpart = {mid, list.length - m};
//
//    ptr_t(Node) lnew;
//    ptr_t(Node) rnew;
//    merge_routine_data mr_data = {lpart, lnew};
//
//    pthread_t thread;
//    const size_t SPAWN_LB = 16;
//    const size_t MAX_THREADS_CNT = 16;
//    bool spawn_thread = list.length > SPAWN_LB && threads_cnt < MAX_THREADS_CNT;
//    if (spawn_thread) {
//        #if defined(PRECISE_GC)
//            int res = thread_create(&thread, nullptr, merge_sort_routine, (void*) &mr_data);
//        #else
//            int res = pthread_create(&thread, nullptr, merge_sort_routine, (void*) &mr_data);
//        #endif
//
//        assert(res == 0);
//        ++threads_cnt;
//    } else {
//        merge_sort(lpart, lnew);
//    }
//    merge_sort(rpart, rnew);
//
//    if (spawn_thread) {
//        void* ret;
//        #if defined(PRECISE_GC)
//            thread_join(thread, &ret);
//        #else
//            pthread_join(thread, &ret);
//        #endif
//
//        lnew = mr_data.res;
//        --threads_cnt;
//    }
//
//    lpart = {lnew, m};
//    rpart = {rnew, list.length - m};
//    res = merge(lpart, rpart);
//}
//
//void* merge_sort_routine(void* arg)
//{
//    merge_routine_data* data = (merge_routine_data*) arg;
//    merge_sort(data->list, data->res);
//    return nullptr;
//}
//
//List create_list(size_t n, int mod)
//{
//    if (n == 0) {
//        return {null_ptr(Node), 0};
//    }
//    size_t length = n;
//    ptr_t(Node) head = new_(Node);
//    ptr_t(Node) it = head;
//    head->data = rand() % mod;
//    --n;
//    while (n > 0) {
//        it->next = new_(Node);
//        it = it->next;
//        it->data = rand() % mod;
//        --n;
//    }
//    return {head, length};
//}
//
//void clear_list(List& list)
//{
//    #ifdef NO_GC
//        ptr_t(Node) it = list.head;
//        while (list.length > 0) {
//            ptr_t(Node) next = it->next;
//            delete_(it);
//            it = next;
//            --list.length;
//        }
//    #endif
//    set_null(list.head);
//    list.length = 0;
//}
//
//NONIUS_BENCHMARK("parallel_merge_sort", [](nonius::chronometer meter)
//{
//    #if defined(PRECISE_GC)
//        gc_init();
//    #elif defined(BDW_GC)
//        GC_INIT();
//        GC_enable_incremental();
//    #endif
//
//    threads_cnt = 1;
//    const size_t LIST_SIZE = 4 * 1024;
//
//    meter.measure([] {
//        List list = create_list(LIST_SIZE, LIST_SIZE);
//        ptr_t(Node) res;
//
//        merge_sort(list, res);
//
//        List sorted_list = {res, LIST_SIZE};
//        clear_list(list);
//        clear_list(sorted_list);
//    });
//
//    ptr_t(Node) sorted = res;
//    for (size_t i = 0; i < LIST_SIZE - 1; ++i) {
////        std::cout << sorted->data << " ";
//        assert(sorted->data <= sorted->next->data);
//        sorted = sorted->next;
//    }
//    std::cout << std::endl << "Threads spawned: " << threads_cnt << std::endl;
//
//    #if defined(BDW_GC)
//        cout << "Completed " << GC_gc_no << " collections" <<endl;
//        cout << "Heap size is " << GC_get_heap_size() << endl;
//    #elif defined(PRECISE_GC)
//        cout << "Completed " << details::gc_garbage_collector::instance().get_gc_cycles_count() << " collections" <<endl;
//        cout << "Heap size is " << details::gc_heap::instance().size() << endl;
//    #endif
//
//});