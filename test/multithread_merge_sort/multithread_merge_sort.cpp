#include <iostream>
#include <thread>
#include <vector>
#include <cmath>
#include "timing.h"
//#include <libprecisegc/libprecisegc.h>
#include "../../sources/libprecisegc.h"
//const int ARRAY_SIZE = 800000;
const int ARRAY_SIZE = 8000;

struct merge_data {
    gc_ptr<int> res;
    gc_ptr<int> arr;
    int begin;
    int end;
};

void merge (gc_ptr<int> res, gc_ptr<int> arr1, int b1, int e1, gc_ptr<int> arr2, int b2, int e2) {
    int i = b1, j = b2, k = 0, s1 = (e1 - b1), s2 = (e2 - b2);
    while (i < e1 && j < e2) {
        res[k++] = arr1[i] < arr2[j] ? arr1[i++] : arr2[j++];
    }
    while (i < e1) { res[k++] = arr1[i++]; }
    while (j < e2) { res[k++] = arr2[j++]; }
}


void* merge_routine(void* arg);

void merge_sort (gc_ptr<int> res, gc_ptr<int> arr, int begin, int end) {
    if (begin >= end) { res[0] = arr[begin]; return; }

    int middle = begin + (end - begin) / 2;
    gc_ptr<int> res1 = gc_new<int>(middle - begin + 1), res2 = gc_new<int>(end - middle);

    if (end - begin > ARRAY_SIZE / 16) {
        pthread_t first;
        pthread_t second;
        merge_data first_data = {res1, arr, begin, middle};
        merge_data second_data = {res2, arr, middle + 1, end};
        thread_create(&first, nullptr, merge_routine, &first_data);
        thread_create(&second, nullptr, merge_routine, &second_data);
        thread_join(first, nullptr);
        thread_join(second, nullptr);
    } else {
        merge_sort(res1, arr, begin, middle);
        merge_sort(res2, arr, middle + 1, end);
    }
    merge (res, res1, 0, middle - begin + 1, res2, 0, end - middle);
}


void* merge_routine(void* arg) {
    merge_data* data = (merge_data*) arg;
    merge_sort(data->res, data->arr, data->begin, data->end);
}

int main(void) {
	init_segregated_storage();
    long tStart, tFinish;
    std::cout << "Merge_sort of array of " << ARRAY_SIZE << " elements:" << std::endl;

    std::cout << "\tCreating         ";
    tStart = currentTime();
    gc_ptr<int> arr = gc_new<int>(ARRAY_SIZE), res = gc_new<int>(ARRAY_SIZE);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = rand() % (ARRAY_SIZE);
    }
    tFinish = currentTime();
    std::cout << elapsedTime(tFinish - tStart) << " msec" << std::endl;

    std::cout << "\tSorting          ";
    tStart = currentTime();
    merge_sort(res, arr, 0, ARRAY_SIZE - 1);
    tFinish = currentTime();
    std::cout << elapsedTime(tFinish - tStart) << " msec" << std::endl;

//     for (int i = 0; i < ARRAY_SIZE; i++)
//         std::cout << res[i] << std::endl;
     return 0;
}