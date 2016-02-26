#ifndef DIPLOMA_INDEX_TREE_H
#define DIPLOMA_INDEX_TREE_H

#include <cstddef>
#include <array>
#include <type_traits>
#include <vector>
#include <memory>

#include "constants.h"
#include "types.h"

#include "any_ptr.h"
#include "lazy_ptr.h"

namespace precisegc { namespace details { namespace allocators {

template <typename T, typename Alloc>
class index_tree
{
public:
    void index(byte* mem, size_t size, T* entry)
    {
        assert(reinterpret_cast<std::uintptr_t>(mem) % PAGE_SIZE == 0);
        byte* mem_end = mem + size;
        for (byte* it = mem; it != mem_end; it += PAGE_SIZE) {
            index_page(it, entry);
        }
    }

    void remove_index(byte* mem, size_t size)
    {
        assert(reinterpret_cast<std::uintptr_t>(mem) % PAGE_SIZE == 0);
        byte* mem_end = mem + size;
        for (byte* it = mem; it != mem_end; it += PAGE_SIZE) {
            remove_page_index(it);
        }
    }

    T* get_entry(byte* mem)
    {

    }

    template <typename U>
    friend class std::unique_ptr;
private:
    static const size_t LEVEL_CNT = 3;
    static const size_t USED_BITS_CNT = POINTER_BITS_CNT - PAGE_BITS_CNT;
    static const size_t LEVEL_BITS_CNT = USED_BITS_CNT / LEVEL_CNT;
    static const size_t FIRST_LEVEL_BITS_CNT = USED_BITS_CNT -  LEVEL_BITS_CNT * (LEVEL_CNT - 1);
    static const size_t LEVEL_SIZE = 1 << LEVEL_BITS_CNT;
    static const size_t FIRST_LEVEL_SIZE = 1 << FIRST_LEVEL_BITS_CNT;

    class tree_level
    {
        struct factory
        {
        public:

            tree_level* create() const
            {
                tree_level* level = nullptr;
                static const size_t size = sizeof(tree_level);
                level = reinterpret_cast<tree_level*>(allocator.allocate(size));
                memset(level, 0, size);
                return level;
            }

            void destroy(tree_level* ptr) const
            {
                static const size_t size = LEVEL_SIZE * sizeof(tree_level);
                allocator.deallocate(reinterpret_cast<byte*>(ptr), size);
            }
        private:
            static Alloc allocator;
        };

    public:
        typedef any_ptr<lazy_ptr<tree_level, factory>> tree_level_ptr;

        tree_level(size_t depth)
            : m_cnt(0)
            , m_shift(POINTER_BITS_CNT - FIRST_LEVEL_BITS_CNT - depth * LEVEL_BITS_CNT)
        {}

        void index(std::uintptr_t page, T* entry, size_t depth)
        {
            size_t ind = calculate_index(page);
            if (depth == LEVEL_CNT - 1) {
                ++m_cnt;
                assert(!m_data[ind]);
                m_data[ind] = entry;
            } else {
                if (!m_data[ind]) {
                    ++m_cnt;
                }
                m_data[ind].as<tree_level>()->index(page, entry, depth + 1);
            }
        }

        void remove_index(std::uintptr_t page, size_t depth)
        {
            size_t ind = calculate_index(page);
            if (depth == LEVEL_CNT - 1) {
                --m_cnt;
                assert(m_data[ind]);
                m_data[ind].reset();
            } else {
                tree_level* next = m_data[ind].as<tree_level>();
                next->remove_index(page, depth + 1);
                if (next->m_cnt == 0) {
                    m_data[ind].reset();
                    --m_cnt;
                }
            }
        }
    private:
        size_t calculate_index(std::uintptr_t page_uintptr)
        {
            return (page_uintptr >> m_shift) & (((size_t) 1 << (LEVEL_BITS_CNT)) - 1);
        }

        size_t m_cnt;
        size_t m_shift;
        tree_level_ptr m_data[LEVEL_SIZE];
    };

    void index_page(byte* page, T* entry)
    {
        std::uintptr_t page_uintptr = reinterpret_cast<std::uintptr_t>(page);
    }

    void remove_page_index(byte* page)
    {
        std::uintptr_t page_uintptr = reinterpret_cast<std::uintptr_t>(page);
    }


    tree_level* m_first_level[FIRST_LEVEL_SIZE];
    Alloc m_allocator;
};

}}}

#endif //DIPLOMA_INDEX_TREE_H
