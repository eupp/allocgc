//
// Created by evagl on 26.03.15.
//
#include <libprecisegc/libprecisegc.h>
void* routine(void* num) {
    for (int i = 0; i < 50; ++i) {
        gc_ptr<int> p[10];
        for (int j = 0; j < 10000; ++j) {
            p[rand() % 10] = gc_new<int, int>(1);
        }
    }
}

constexpr int NUM = 6;

int main() {
    pthread_t threads[NUM];

    for (int i = 0; i < NUM; ++i) {
        thread_create(&threads[i], nullptr, routine, nullptr);
    }
    void* ret;
    for (int i = 0; i < NUM; ++i) {
        thread_join(threads[i], &ret);
    }
    return 0;
}