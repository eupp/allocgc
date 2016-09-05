#ifndef DIPLOMA_INDEX_TREE_H
#define DIPLOMA_INDEX_TREE_H

#include <cstddef>
#include <cstring>
#include <cassert>
#include <array>
#include <atomic>
#include <type_traits>
#include <vector>
#include <iterator>
#include <memory>
#include <mutex>
#include <limits>

#include <boost/integer/static_min_max.hpp>

#include <libprecisegc/details/constants.hpp>
#include <libprecisegc/details/types.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/allocators/freelist_pool_chunk.hpp>
#include <libprecisegc/details/allocators/default_allocator.hpp>
#include <libprecisegc/details/allocators/intrusive_list_pool_allocator.hpp>

namespace precisegc { namespace details {

namespace internals {
struct index_tree_access;

struct splitter
{
    static const size_t LEVEL_CNT               = 3;

    static const size_t USED_BITS_CNT           = POINTER_BITS_USED - PAGE_BITS_CNT;
    static const size_t LEVEL_BITS_CNT          = USED_BITS_CNT / LEVEL_CNT;
    static const size_t LAST_LEVEL_BITS_CNT     = USED_BITS_CNT - (LEVEL_CNT - 1) * LEVEL_BITS_CNT;

    static const size_t LEVEL_SIZE              = (size_t) 1 << LEVEL_BITS_CNT;
    static const size_t LAST_LEVEL_SIZE         = (size_t) 1 << LAST_LEVEL_BITS_CNT;

    typedef unsigned short idx_t;

    static_assert(std::numeric_limits<idx_t>::max() >= boost::static_unsigned_max<LEVEL_SIZE, LAST_LEVEL_SIZE>::value,
                  "idx_t type is not big enough");

    typedef std::array<idx_t, LEVEL_CNT> idxs_t;

    static idxs_t split(byte* ptr)
    {
        std::uintptr_t x = reinterpret_cast<std::uintptr_t>(ptr);
        x &= ((std::uintptr_t) 1 << (POINTER_BITS_USED)) - 1;

        static const size_t shifts[] = {
                  POINTER_BITS_USED - LAST_LEVEL_BITS_CNT
                , POINTER_BITS_USED - LAST_LEVEL_BITS_CNT - LEVEL_BITS_CNT
                , POINTER_BITS_USED - LAST_LEVEL_BITS_CNT - 2 * LEVEL_BITS_CNT
        };

        idxs_t idxs;
        idxs[0] = x >> shifts[0];
        for (size_t i = 1; i < LEVEL_CNT; ++i) {
            idxs[i] = (x >> shifts[i]) & (((std::uintptr_t) 1 << (LEVEL_BITS_CNT)) - 1);
        }

        for (int i = 0; i < idxs.size() - 1; ++i) {
            assert(idxs[i] < LEVEL_SIZE);
        }
        assert(idxs.back() < LAST_LEVEL_SIZE);

        return idxs;
    }
};
}

template <typename T>
class index_tree : private utils::noncopyable, private utils::nonmovable
{
public:
    typedef T entry_type;

    index_tree()
    {}

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

    friend class internals::index_tree_access;
private:
    typedef internals::splitter::idx_t  idx_t;
    typedef internals::splitter::idxs_t idxs_t;

    template <typename Level>
    class level_factory
    {
        class level_deleter
        {
        public:
            void operator()(Level* level) const
            {
                destroy(level);
            }
        };
    public:
        static std::unique_ptr<Level, level_deleter> create()
        {
            std::unique_lock<std::mutex> lock(mutex);
            auto ptr = std::unique_ptr<Level, level_deleter>(reinterpret_cast<Level*>(level_pool.allocate(sizeof(Level))));
            lock.unlock();
            new (ptr.get()) Level();
            return ptr;
        }

        static void destroy(Level* level)
        {
            level->~Level();
            std::lock_guard<std::mutex> lock(mutex);
            level_pool.deallocate(reinterpret_cast<byte*>(level), sizeof(Level));
        }
    private:
        static const size_t LEVELS_IN_BLOCK = 32;

        typedef allocators::intrusive_list_pool_allocator<
                allocators::freelist_pool_chunk,
                allocators::default_allocator,
                allocators::block_size_for<allocators::freelist_pool_chunk>(sizeof(Level), LEVELS_IN_BLOCK)
            > level_pool_t;

        static level_pool_t level_pool;
        static std::mutex mutex;
    };

    template <typename Level>
    class internal_level : private utils::noncopyable, private utils::nonmovable
    {
    public:
        internal_level()
        {
            for (auto& hdl: m_data) {
                hdl.m_ptr.store(&null, std::memory_order_release);
                hdl.m_cnt.store(0, std::memory_order_release);
            }
        }

        void index(idxs_t::iterator idx, T* entry)
        {
            idxs_t::iterator next_idx = std::next(idx);

            m_data[*idx].m_cnt.fetch_add(1, std::memory_order_acq_rel);

            Level* next_level = m_data[*idx].m_ptr.load(std::memory_order_acquire);
            if (next_level == &null) {
                auto new_level = level_factory<Level>::create();
                new_level->index(next_idx, entry);
                if (m_data[*idx].m_ptr.compare_exchange_strong(next_level, new_level.get(), std::memory_order_acq_rel)) {
                    new_level.release();
                } else {
                    next_level->index(next_idx, entry);
                }
            } else {
                next_level->index(next_idx, entry);
            }
        }

        void remove_index(idxs_t::iterator idx)
        {
            idxs_t::iterator next_idx = std::next(idx);

            Level* next_level = m_data[*idx].m_ptr.load(std::memory_order_acquire);
            next_level->remove_index(next_idx);
            if (m_data[*idx].m_cnt.fetch_sub(1, std::memory_order_acq_rel) == 1) {
                m_data[*idx].m_ptr.store(&null, std::memory_order_release);
                level_factory<Level>::destroy(next_level);
            }
        }

        T* get(idxs_t::iterator idx) const
        {
            Level* next_level = m_data[*idx].m_ptr.load(std::memory_order_acquire);
            return next_level->get(++idx);
        }

        void clear()
        {
            for (auto& hdl: m_data) {
                Level* level = hdl.m_ptr.load(std::memory_order_acquire);
                if (level != &null) {
                    level->clear();
                    level_factory<Level>::destroy(level);
                }
            }
        }

        static const size_t LEVEL_SIZE = internals::splitter::LEVEL_SIZE;

        friend class internals::index_tree_access;
    private:
        static Level null;

        struct handle
        {
            std::atomic<Level*> m_ptr;
            std::atomic<size_t> m_cnt;
        };

        typedef std::array<handle, LEVEL_SIZE> array_t;

        // debug stuff
        size_t get_count(idx_t idx) const
        {
            return m_data[idx].m_cnt.load();
        }

        // debug stuff
        Level* get_level(idx_t idx) const
        {
            return m_data[idx].m_ptr.load();
        }

        array_t m_data;
    };

    class last_level : private utils::noncopyable, private utils::nonmovable
    {
    public:
        last_level()
        {
            for (auto& ptr: m_data) {
                ptr.store(nullptr, std::memory_order_release);
            }
        }

        void index(idxs_t::iterator idx, T* entry)
        {
            assert(!m_data[*idx].load(std::memory_order_acquire));
            m_data[*idx].store(entry, std::memory_order_release);
        }

        void remove_index(idxs_t::iterator idx)
        {
            assert(m_data[*idx].load(std::memory_order_acquire));
            m_data[*idx].store(nullptr, std::memory_order_release);
        }

        T* get(idxs_t::iterator idx) const
        {
            return m_data[*idx].load(std::memory_order_acquire);
        }

        void clear()
        {
            return;
        }

        static const size_t LEVEL_SIZE = internals::splitter::LAST_LEVEL_SIZE;
    private:
        typedef std::array<std::atomic<T*>, LEVEL_SIZE> array_t;

        array_t m_data;
    };

    void index_page(byte* page, T* entry)
    {
        idxs_t idxs = internals::splitter::split(page);
        m_first_level.index(idxs.begin(), entry);
    }

    void remove_page_index(byte* page)
    {
        idxs_t idxs = internals::splitter::split(page);
        m_first_level.remove_index(idxs.begin());
    }

    T* get_page_entry(byte* page)
    {
        idxs_t idxs = internals::splitter::split(page);
        return m_first_level.get(idxs.begin());
    }

    void clear()
    {
        m_first_level.clear();
    }

    typedef internal_level<internal_level<last_level>> first_level_t;

    first_level_t m_first_level;
};

namespace internals {
struct index_tree_access
{
    template <typename T, typename Level>
    using internal_level = typename index_tree<T>::template internal_level<Level>;

    template <typename T>
    using last_level = typename index_tree<T>::last_level;

    typedef splitter::idx_t  idx_t;
    typedef splitter::idxs_t idxs_t;

    template <typename T, typename Level>
    static size_t get_level_count(const internal_level<T, Level>& level, idx_t idx)
    {
        return level.get_count(idx);
    };

    template <typename T, typename Level>
    static Level* get_level(const internal_level<T, Level>& level, idx_t idx)
    {
        return level.get_level(idx);
    };
};
}

template<typename T>
template<typename Level>
Level index_tree<T>::internal_level<Level>::null{};

template <typename T>
template <typename Level>
typename index_tree<T>::template level_factory<Level>::level_pool_t index_tree<T>::level_factory<Level>::level_pool{};

template <typename T>
template <typename Level>
std::mutex index_tree<T>::level_factory<Level>::mutex{};

}}

#endif //DIPLOMA_INDEX_TREE_H
