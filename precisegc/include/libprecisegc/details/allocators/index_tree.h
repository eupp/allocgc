#ifndef DIPLOMA_INDEX_TREE_H
#define DIPLOMA_INDEX_TREE_H

#include <cstddef>
#include <cstring>
#include <cassert>
#include <array>
#include <type_traits>
#include <vector>
#include <memory>
#include <mutex>
#include <unordered_map>

#include "constants.h"
#include "types.h"

#include "any_ptr.h"
#include "../util.h"

namespace precisegc { namespace details { namespace allocators {

template <typename T, typename Alloc>
class index_tree : private ebo<Alloc>, private noncopyable, private nonmovable
{
    static const size_t CACHE_SIZE = 128;
    typedef std::unordered_map<byte*, T*> cache_t;
public:
    typedef T entry_type;

    index_tree()
    {
//        m_cache.reserve(CACHE_SIZE);
    }

    ~index_tree()
    {
        clear();
    }

    void index(byte* mem, size_t size, T* entry)
    {
        assert(reinterpret_cast<std::uintptr_t>(mem) % PAGE_SIZE == 0);
        assert(size % PAGE_SIZE == 0);
        byte* mem_end = mem + size;
        for (byte* it = mem; it != mem_end; it += PAGE_SIZE) {
            index_page(it, entry);
        }
    }

    void remove_index(byte* mem, size_t size)
    {
        assert(reinterpret_cast<std::uintptr_t>(mem) % PAGE_SIZE == 0);
        assert(size % PAGE_SIZE == 0);
        byte* mem_end = mem + size;
        for (byte* it = mem; it != mem_end; it += PAGE_SIZE) {
            remove_page_index(it);
        }
    }

    T* get_entry(byte* mem)
    {
        return get_page_entry(mem);
    }

    void reset_cache()
    {
//        m_cache.clear();
    }

    Alloc& get_allocator()
    {
        return this->template get_base<Alloc>();
    }

    const Alloc& get_allocator() const
    {
        return this->template get_base<Alloc>();
    }

    const Alloc& get_const_allocator() const
    {
        return this->template get_base<Alloc>();
    }
private:
    static const size_t LEVEL_CNT = 3;
    static const size_t USED_BITS_CNT = POINTER_BITS_CNT - PAGE_BITS_CNT;
    static const size_t LEVEL_BITS_CNT = USED_BITS_CNT / LEVEL_CNT;
    static const size_t FIRST_LEVEL_BITS_CNT = USED_BITS_CNT -  LEVEL_BITS_CNT * (LEVEL_CNT - 1);
    static const size_t LEVEL_SIZE = 1 << LEVEL_BITS_CNT;
    static const size_t FIRST_LEVEL_SIZE = 1 << FIRST_LEVEL_BITS_CNT;

    typedef std::mutex mutex_type;

    struct tree_third_level
    {
        T* m_data[LEVEL_SIZE] = {0};
        mutex_type m_mutex;
    };

    struct tree_second_level
    {
        tree_third_level* m_data[LEVEL_SIZE] = {0};
        size_t m_cnts[LEVEL_SIZE] = {0};
        mutex_type m_mutex;
    };

    struct tree_first_level
    {
        tree_second_level* m_data[FIRST_LEVEL_SIZE] = {0};
        size_t m_cnts[FIRST_LEVEL_SIZE] = {0};
        mutex_type m_mutex;
    };

    typedef std::array<size_t, LEVEL_CNT> level_indices;

    level_indices get_indicies(byte* page) const
    {
        std::uintptr_t page_uintptr = reinterpret_cast<std::uintptr_t>(page);
        size_t shift = POINTER_BITS_CNT - FIRST_LEVEL_BITS_CNT;
        level_indices ind;
        ind[0] = page_uintptr >> shift;
        for (size_t i = 1; i < LEVEL_CNT; ++i) {
            shift -= LEVEL_BITS_CNT;
            ind[i] = (page_uintptr >> shift) & (((size_t) 1 << (LEVEL_BITS_CNT)) - 1);
        }
        return ind;
    }

    void index_page(byte* page, T* entry)
    {
//        if (m_cache.size() < CACHE_SIZE) {
//            m_cache[page] = entry;
//        }

        level_indices ind;
        ind = get_indicies(page);
        std::lock_guard<mutex_type> lock(m_mutex);

        tree_second_level* second_level = nullptr;
        {
//            std::lock_guard<mutex_type> lock(m_first_level.m_mutex);
            ++m_first_level.m_cnts[ind[0]];
            if (!m_first_level.m_data[ind[0]]) {
                m_first_level.m_data[ind[0]] = allocate<tree_second_level>();
            }
            second_level = m_first_level.m_data[ind[0]];
        }

        tree_third_level* third_level = nullptr;
        {
//            std::lock_guard<mutex_type> lock(second_level->m_mutex);
            ++second_level->m_cnts[ind[1]];
            if (!second_level->m_data[ind[1]]) {
                second_level->m_data[ind[1]] = allocate<tree_third_level>();
            }
            third_level = second_level->m_data[ind[1]];
        }

        {
//            std::lock_guard<mutex_type> lock(third_level->m_mutex);
            third_level->m_data[ind[2]] = entry;
        }
    }

    void remove_page_index(byte* page)
    {
//        m_cache.erase(page);

        level_indices ind;
        ind = get_indicies(page);
        std::lock_guard<mutex_type> lock(m_mutex);

        tree_second_level* second_level = nullptr;
        size_t second_cnt = 0;
        {
//            std::lock_guard<mutex_type> lock(m_first_level.m_mutex);
            second_level = m_first_level.m_data[ind[0]];
            second_cnt = --m_first_level.m_cnts[ind[0]];
            if (second_cnt == 0) {
                m_first_level.m_data[ind[0]] = nullptr;
            }
        }
        assert(second_level);

        tree_third_level* third_level = nullptr;
        size_t third_cnt = 0;
        {
//            std::lock_guard<mutex_type> lock(second_level->m_mutex);
            third_level = second_level->m_data[ind[1]];
            third_cnt = --second_level->m_cnts[ind[1]];
            if (third_cnt == 0) {
                second_level->m_data[ind[1]] = nullptr;
            }
        }
        assert(third_level);

        {
//            std::lock_guard<mutex_type> lock(third_level->m_mutex);
            third_level->m_data[ind[2]] = nullptr;
        }

        if (second_cnt == 0) {
            deallocate(second_level);
        }

        if (third_cnt == 0) {
            deallocate(third_level);
        }
    }

    T* get_page_entry(byte* page)
    {
//        T*& cached_entry = m_cache[page];
//        if (cached_entry) {
//            return cached_entry;
//        }

        level_indices ind;
        ind = get_indicies(page);
        std::lock_guard<mutex_type> lock(m_mutex);

        tree_second_level* second_level = nullptr;
        {
//            std::lock_guard<mutex_type> lock(m_first_level.m_mutex);
            second_level = m_first_level.m_data[ind[0]];
        }
        if (!second_level) {
            return nullptr;
        }

        tree_third_level* third_level = nullptr;
        {
//            std::lock_guard<mutex_type> lock(second_level->m_mutex);
            third_level = second_level->m_data[ind[1]];
        }
        if (!third_level) {
            return nullptr;
        }

        {
//            std::lock_guard<mutex_type> lock(third_level->m_mutex);
            T* entry = third_level->m_data[ind[2]];
//            if (m_cache.size() < CACHE_SIZE) {
//                cached_entry = entry;
//            }
            return entry;
        }
    }

    void clear()
    {
        for (auto second_level: m_first_level.m_data) {
            if (second_level) {
                for (auto third_level: second_level->m_data) {
                    if (third_level) {
                        deallocate(third_level);
                    }
                }
                deallocate(second_level);
            }
        }
    }

    template <typename U>
    U* allocate()
    {
        U* p = reinterpret_cast<U*>(get_allocator().allocate(sizeof(U)));
        new (p) U();
        return p;
    }

    template <typename U>
    void deallocate(U* p)
    {
        p->~U();
        get_allocator().deallocate(reinterpret_cast<byte*>(p), sizeof(U));
    }

    tree_first_level m_first_level;
//    cache_t m_cache;
    mutex_type m_mutex;
};

}}}

#endif //DIPLOMA_INDEX_TREE_H
