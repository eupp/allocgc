#include "deref_roots.h"
#include "go.h"
#include "threading.h"
#include <assert.h>
#include <sys/mman.h>

#define deref_mark(h) (h->p |= 1)
#define deref_unmark(h) (h->p &= ~1)
#define deref_is_marked(h) (h->p & 1 != 0)
#define plain_pointer(h) ((void*)(h->p & ~1))
#define contains(h, p) (plain_pointer(h) <= p && p <= h->end)

struct root_handler {
    size_t p;
    void* end;
    root_handler * next;
};
static constexpr size_t N = 100003;
static root_handler *roots[N];
static root_handler* free_list = nullptr;
static pthread_mutex_t deref_mutex = PTHREAD_MUTEX_INITIALIZER;
static size_t mmap_count = 4096 * 128;
static size_t count = 0;
static size_t total_count = 0;
static size_t expand_count = 0;
static size_t expand_factor = 8;

void register_dereferenced_root(void* root, size_t size) {
    if(!root) {
        return;
    }
    size_t hash = ((size_t)root >> 3) % N;
    pthread_mutex_lock(&deref_mutex);
    if (free_list == nullptr) {
        void* space = mmap(nullptr, mmap_count * sizeof(root_handler), PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        assert(space != MAP_FAILED);
        void* space_end = space + mmap_count * sizeof(root_handler);
        for (root_handler* curr = (root_handler*) space; curr < space_end; ++curr) {
            curr->next = free_list;
            free_list = curr;
        }
        total_count += mmap_count;
    }
    root_handler * data = free_list;
    free_list = free_list->next;
    data->next = roots[hash];
    data->p = (size_t) root;
    data->end = ((char*)root) + size;
    roots[hash] = data;
    count++;
    bool need_gc = false;
    if (count == total_count) {
        expand_count++;
        need_gc = expand_count % expand_factor == 0;
    }
    pthread_mutex_unlock(&deref_mutex);
    if (need_gc) {
        gc(false);
        expand_count = 0;
        if (expand_factor != 1) {
            expand_factor /= 2;
        }
    }
}

void mark_dereferenced_root(void* root, bool full_gc) {
    if (!root || ((size_t) root) % 2 != 0) {
        return;
    }
    size_t hash = ((size_t)root >> 3) % N;
    root_handler* curr = roots[hash];
    while (curr) {
        if (contains(curr, root)) {
            if (!deref_is_marked(curr)) {
                deref_mark(curr);
                if (full_gc) {
                    go(plain_pointer(curr), true);
                }
            }
            return;
        }
        curr = curr->next;
    }
}

void sweep_dereferenced_roots() {
    for (size_t i = 0; i < N; ++i) {
        root_handler *curr = roots[i];
        root_handler *prev = nullptr;
        while (curr) {
            if (!deref_is_marked(curr)) {
                root_handler *tmp = curr->next;
                if (!prev) {
                    roots[i] = curr->next;
                } else {
                    prev->next = curr->next;
                }
                curr->next = free_list;
                free_list = curr;
                curr = tmp;
                count--;
            } else {
                deref_unmark(curr);
                prev = curr;
                curr = curr->next;
            }
        }
    }
}



