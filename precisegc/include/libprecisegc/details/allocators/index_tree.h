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
#include "../util.h"

namespace precisegc { namespace details { namespace allocators {

template <typename T, typename Alloc>
class index_tree: private noncopyable, private nonmovable
{
public:
    index_tree()
    {
        memset(m_first_level, 0, FIRST_LEVEL_SIZE * sizeof(any_ptr));
    }

    ~index_tree()
    {
        clear();
    }

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
        return get_page_entry(mem);
    }
private:
    static const size_t LEVEL_CNT = 3;
    static const size_t USED_BITS_CNT = POINTER_BITS_CNT - PAGE_BITS_CNT;
    static const size_t LEVEL_BITS_CNT = USED_BITS_CNT / LEVEL_CNT;
    static const size_t FIRST_LEVEL_BITS_CNT = USED_BITS_CNT -  LEVEL_BITS_CNT * (LEVEL_CNT - 1);
    static const size_t LEVEL_SIZE = 1 << LEVEL_BITS_CNT;
    static const size_t FIRST_LEVEL_SIZE = 1 << FIRST_LEVEL_BITS_CNT;

    struct tree_level
    {
        size_t m_cnt;
        any_ptr m_data[LEVEL_SIZE];
    };

    struct path_element
    {
        any_ptr& get_element()
        {
            return m_level[m_ind];
        }

        any_ptr* m_level;
        size_t m_ind;
    };

    typedef std::array<path_element, LEVEL_CNT> tree_path;

    enum class allocation_option
    {
        ALLOCATE,
        NO_ALLOCATE
    };

    tree_path traverse(byte* page, allocation_option alloc_opt)
    {
        tree_path path;
        memset(&path, 0, sizeof(tree_path));
        std::uintptr_t page_uintptr = reinterpret_cast<std::uintptr_t>(page);

        size_t shift = POINTER_BITS_CNT - FIRST_LEVEL_BITS_CNT;
        any_ptr* level = m_first_level;
        size_t ind = page_uintptr >> shift;

        path[0].m_level = level;
        path[0].m_ind = ind;

        if (!level[ind]) {
            if (alloc_opt == allocation_option::ALLOCATE) {
                level[ind].reset(allocate_tree_level());
            } else {
                return path;
            }
        }

        for (size_t i = 1; i < LEVEL_CNT; ++i) {
            tree_level* prev = level[ind].as<tree_level>();
            level = prev->m_data;

            shift -= LEVEL_BITS_CNT;
            ind = (page_uintptr >> shift) & (((size_t) 1 << (LEVEL_BITS_CNT)) - 1);

            path[i].m_level = level;
            path[i].m_ind = ind;

            if (!level[ind] && i < LEVEL_CNT - 1) {
                if (alloc_opt == allocation_option::ALLOCATE) {
                    level[ind].reset(allocate_tree_level());
                    ++prev->m_cnt;
                } else {
                    return path;
                }
            }
        }

        return path;
    }

    void index_page(byte* page, T* entry)
    {
        tree_path path = traverse(page, allocation_option::ALLOCATE);

        any_ptr& leaf = path[LEVEL_CNT - 1].get_element();
        assert(!leaf);
        leaf.reset(entry);

        any_ptr& inner = path[LEVEL_CNT - 2].get_element();
        ++inner.as<tree_level>()->m_cnt;
    }

    void remove_page_index(byte* page)
    {
        tree_path path = traverse(page, allocation_option::ALLOCATE);

        any_ptr& leaf = path[LEVEL_CNT - 1].get_element();
        assert(leaf);
        leaf.reset();

        any_ptr& inner = path[LEVEL_CNT - 2].get_element();
        --inner.as<tree_level>()->m_cnt;

        for (size_t i = LEVEL_CNT - 2; i > 0; --i) {
            any_ptr& inner = path[i].get_element();
            tree_level* level = inner.as<tree_level>();
            if (level->m_cnt == 0) {
                inner.reset();
                deallocate_tree_level(level);

                any_ptr& prev = path[i-1].get_element();
                --prev.as<tree_level>()->m_cnt;
            }
        }
    }

    T* get_page_entry(byte* page)
    {
        tree_path path = traverse(page, allocation_option::NO_ALLOCATE);

        if (path[LEVEL_CNT - 1].m_level) {
            any_ptr& leaf = path[LEVEL_CNT - 1].get_element();
            return leaf.as<T>();
        } else {
            return nullptr;
        }
    }

    void clear()
    {
        any_ptr* level = m_first_level;
        for (size_t i = 0; i < FIRST_LEVEL_SIZE; ++i) {
            if (level[i]) {
                clear_inner_level(level[i], 1);
            }
        }
    }

    void clear_inner_level(any_ptr ptr, size_t depth)
    {
        tree_level* level = ptr.as<tree_level>();
        for (size_t i = 0; i < LEVEL_SIZE; ++i) {
            if (level->m_data[i] && depth < LEVEL_CNT - 1) {
                clear_inner_level(level->m_data[i], depth + 1);
            }
        }
        deallocate_tree_level(level);
    }

    tree_level* allocate_tree_level()
    {
        tree_level* level = nullptr;
        static const size_t size = sizeof(tree_level);
        level = reinterpret_cast<tree_level*>(m_allocator.allocate(size));
        memset(level, 0, size);
        return level;
    }

    void deallocate_tree_level(tree_level* level)
    {
        static const size_t size = LEVEL_SIZE * sizeof(tree_level);
        m_allocator.deallocate(reinterpret_cast<byte*>(level), size);
    }


    any_ptr m_first_level[FIRST_LEVEL_SIZE];
    Alloc m_allocator;
};

}}}

#endif //DIPLOMA_INDEX_TREE_H
