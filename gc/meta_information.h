#pragma once
#include <pthread.h>
#include <sys/mman.h>
#include <assert.h>
#include "tlvars.h"

struct meta_holder{
    size_t* clMeta;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
};

template <typename T>
meta_holder* get_meta_holder() {
    static meta_holder holder;
    return &holder;
}

template <typename T>
size_t* get_meta() {
    meta_holder* holder = get_meta_holder<T>();
    pthread_mutex_lock(&holder->mutex);
    size_t* result = holder->clMeta;
    pthread_mutex_unlock(&holder->mutex);
    return result;
}

template <typename T>
size_t* create_meta(tlvars* flags) {
    size_t count = flags->offsets.size();
    size_t* data = (size_t *) mmap(0, sizeof(size_t) * (count + 2), PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    assert(data != MAP_FAILED);
    data[0] = sizeof(T);
    data[1] = count;
    for (int i = 0; i < count; ++i) {
        data[i + 2] = flags->offsets[i];
    }
    meta_holder* holder = get_meta_holder<T>();
    pthread_mutex_lock(&holder->mutex);
    holder->clMeta = data;
    pthread_mutex_unlock(&holder->mutex);
    return data;
}