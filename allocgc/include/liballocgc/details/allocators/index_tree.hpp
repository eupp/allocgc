#ifndef ALLOCGC_INDEX_TREE_H
#define ALLOCGC_INDEX_TREE_H

#include <cstddef>
#include <cassert>
#include <array>
#include <atomic>
#include <type_traits>
#include <iterator>
#include <memory>
#include <limits>

#include <boost/integer/static_min_max.hpp>

#include <liballocgc/gc_common.hpp>
#include <liballocgc/details/constants.hpp>
#include <liballocgc/details/utils/utility.hpp>
#include <liballocgc/details/allocators/pool.hpp>
#include <liballocgc/details/allocators/memory_descriptor.hpp>

namespace allocgc { namespace details { namespace allocators {

namespace internals {
struct index_tree_access;

struct splitter
{
    static const size_t LEVEL_CNT               = 3;

    static const size_t USED_BITS_CNT           = POINTER_BITS_USED - PAGE_SIZE_LOG2;
    static const size_t LEVEL_BITS_CNT          = USED_BITS_CNT / LEVEL_CNT;
    static const size_t LAST_LEVEL_BITS_CNT     = USED_BITS_CNT - (LEVEL_CNT - 1) * LEVEL_BITS_CNT;

    static const size_t LEVEL_SIZE              = (size_t) 1 << LEVEL_BITS_CNT;
    static const size_t LAST_LEVEL_SIZE         = (size_t) 1 << LAST_LEVEL_BITS_CNT;

    typedef unsigned short idx_t;

    static_assert(std::numeric_limits<idx_t>::max() >= boost::static_unsigned_max<LEVEL_SIZE, LAST_LEVEL_SIZE>::value,
                  "idx_t type is not big enough");

    typedef std::array<idx_t, LEVEL_CNT> idxs_t;

    static idxs_t split(const byte* ptr)
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

        for (size_t i = 0; i < idxs.size() - 1; ++i) {
            assert(idxs[i] < LEVEL_SIZE);
        }
        assert(idxs.back() < LAST_LEVEL_SIZE);

        return idxs;
    }
};
}

class index_tree : private utils::noncopyable, private utils::nonmovable
{
public:
    index_tree()
    {
        assert(std::atomic<memory_descriptor>().is_lock_free());
    }

    ~index_tree()
    {
        clear();
    }

    static void init()
    {
        init_impl();
    }

    void clear()
    {
        clear_impl();
    }

    void index(const byte* mem, size_t size, memory_descriptor descriptor)
    {
        assert(reinterpret_cast<std::uintptr_t>(mem) % PAGE_SIZE == 0);
//        assert(size % PAGE_SIZE == 0);
        const byte* mem_end = mem + size;
        for (const byte* it = mem; it < mem_end; it += PAGE_SIZE) {
            add_page_to_index(it, descriptor);
        }
    }

    void deindex(const byte* mem, size_t size)
    {
        assert(reinterpret_cast<std::uintptr_t>(mem) % PAGE_SIZE == 0);
//        assert(size % PAGE_SIZE == 0);
        const byte* mem_end = mem + size;
        for (const byte* it = mem; it < mem_end; it += PAGE_SIZE) {
            remove_page_from_index(it);
        }
    }

    inline memory_descriptor get_descriptor(const byte* mem) const
    {
        return index_page(mem);
    }

    size_t size()
    {
        return m_first_level.size();
    }

    friend struct internals::index_tree_access;
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
                instance().destroy(level);
            }
        };
    public:
        static level_factory& instance()
        {
            static level_factory factory;
            return factory;
        }

        template <typename... Args>
        std::unique_ptr<Level, level_deleter> create(Args&&... args)
        {
            return std::unique_ptr<Level, level_deleter>(m_pool.create(std::forward<Args>(args)...));
        }

        void destroy(Level* level)
        {
            m_pool.destroy(level);
        }
    private:
        allocators::pool<Level, std::mutex> m_pool;
    };

    template <typename Level>
    class internal_level : private utils::noncopyable, private utils::nonmovable
    {
    public:
        internal_level()
        {
            for (auto& hdl: m_data) {
                hdl.m_ptr.store(null(), std::memory_order_release);
                hdl.m_cnt.store(0, std::memory_order_release);
            }
        }

        internal_level(const internal_level& other)
            : m_data(other.m_data)
        {}

        void index(idxs_t::iterator idx, memory_descriptor entry)
        {
            idxs_t::iterator next_idx = std::next(idx);

            m_data[*idx].m_cnt.fetch_add(1, std::memory_order_acq_rel);

            Level* next_level = m_data[*idx].m_ptr.load(std::memory_order_acquire);
            if (next_level == null()) {
                auto new_level = level_factory<Level>::instance().create(*null());
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
                m_data[*idx].m_ptr.store(null(), std::memory_order_release);
                level_factory<Level>::instance().destroy(next_level);
            }
        }

        memory_descriptor get(idxs_t::iterator idx) const
        {
            Level* next_level = m_data[*idx].m_ptr.load(std::memory_order_acquire);
            return next_level->get(++idx);
        }

        void clear()
        {
            for (auto& hdl: m_data) {
                Level* level = hdl.m_ptr.load(std::memory_order_acquire);
                if (level != null()) {
                    level->clear();
                    level_factory<Level>::instance().destroy(level);
                }
            }
        }

        size_t size() const
        {
            size_t sz = 0;
            for (auto& handle: m_data) {
                Level* level = handle.m_ptr.load(std::memory_order_acquire);
                if (level != null()) {
                    sz += level->size();
                }
            }
            return sz + sizeof(internal_level);
        }

        static Level* null()
        {
            static Level null;
            return &null;
        }

        static const size_t LEVEL_SIZE = internals::splitter::LEVEL_SIZE;

        friend struct internals::index_tree_access;
    private:
        struct handle
        {
            handle() = default;

            handle(const handle& other)
                : m_ptr(other.m_ptr.load(std::memory_order_relaxed))
                , m_cnt(other.m_cnt.load(std::memory_order_relaxed))
            {}

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
            for (auto& hdl: m_data) {
                hdl.m_ptr.store(nullptr, std::memory_order_release);
            }
        }

        last_level(const last_level& other)
            : m_data(other.m_data)
        {}

        void index(idxs_t::iterator idx, memory_descriptor descriptor)
        {
            assert(!m_data[*idx].m_ptr.load(std::memory_order_acquire));
            m_data[*idx].m_ptr.store(descriptor, std::memory_order_release);
        }

        void remove_index(idxs_t::iterator idx)
        {
            assert(m_data[*idx].m_ptr.load(std::memory_order_acquire));
            m_data[*idx].m_ptr.store(nullptr, std::memory_order_release);
        }

        memory_descriptor get(idxs_t::iterator idx) const
        {
            return m_data[*idx].m_ptr.load(std::memory_order_acquire);
        }

        void clear()
        {
            return;
        }

        size_t size() const
        {
            return sizeof(last_level);
        }

        static const size_t LEVEL_SIZE = internals::splitter::LAST_LEVEL_SIZE;
    private:
        struct handle
        {
            handle() = default;

            handle(const handle& other)
                : m_ptr(other.m_ptr.load(std::memory_order_relaxed))
            {}

            std::atomic<memory_descriptor> m_ptr;
        };

        typedef std::array<handle, LEVEL_SIZE> array_t;

        array_t m_data;
    };

    typedef internal_level<internal_level<last_level>> first_level_t;
    typedef internal_level<last_level> second_level_t;
    typedef last_level third_level_t;

    static void init_impl()
    {
        first_level_t::null();
        second_level_t::null();

        level_factory<second_level_t>::instance();
        level_factory<third_level_t>::instance();
    }

    void add_page_to_index(const byte* page, memory_descriptor entry)
    {
        idxs_t idxs = internals::splitter::split(page);
        m_first_level.index(idxs.begin(), entry);
    }

    void remove_page_from_index(const byte* page)
    {
        idxs_t idxs = internals::splitter::split(page);
        m_first_level.remove_index(idxs.begin());
    }

    memory_descriptor index_page(const byte* page) const
    {
        idxs_t idxs = internals::splitter::split(page);
        return m_first_level.get(idxs.begin());
    }

    void clear_impl()
    {
        m_first_level.clear();
    }

    first_level_t m_first_level;
};

namespace internals {
struct index_tree_access
{
    template <typename Level>
    using internal_level = typename index_tree::template internal_level<Level>;

    using last_level = typename index_tree::last_level;

    typedef splitter::idx_t  idx_t;
    typedef splitter::idxs_t idxs_t;

    template <typename Level>
    static size_t get_level_count(const internal_level<Level>& level, idx_t idx)
    {
        return level.get_count(idx);
    };

    template <typename Level>
    static Level* get_level(const internal_level<Level>& level, idx_t idx)
    {
        return level.get_level(idx);
    };
};
}

}}}

#endif //ALLOCGC_INDEX_TREE_H
