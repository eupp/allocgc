#ifndef DIPLOMA_REMSET_HPP
#define DIPLOMA_REMSET_HPP

#include <unordered_map>
#include <unordered_set>
#include <thread>
#include <mutex>

#include <libprecisegc/details/collectors/managed_object.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details { namespace collectors {

class remset : private utils::noncopyable, private utils::nonmovable
{
    typedef std::unordered_set<byte*> hashtable_t;
public:
    typedef hashtable_t::const_iterator const_iterator;

    remset() = default;

    void  add(byte* ptr);
    byte* get();

    void clear();
    void flush_buffers();

    size_t size() const;
    bool empty() const;

    // we use iterators here but it isn't best option,
    // because we don't hold lock on hash table while we are tracing data through these iterators;
    // however, because we do tracing only during stop-the-world it's ok;
    // but we still need more consistent lock strategy
    const_iterator begin() const;
    const_iterator end() const;
private:
    class sequential_store_buffer : private utils::noncopyable, private utils::nonmovable
    {
    public:
        typedef byte* const* const_iterator;

        sequential_store_buffer();

        void push(byte* ptr);
        void clear();

        bool is_full() const;

        const_iterator begin() const;
        const_iterator end() const;
    private:
        friend class remset;

        static const size_t SSB_SIZE = 64;

        byte* m_data[SSB_SIZE];
        size_t m_size;
    };

    typedef sequential_store_buffer ssb_t;
    typedef std::unordered_map<std::thread::id, ssb_t> ssb_map_t;
    typedef std::mutex mutex_t;

    ssb_t& get_ssb();
    void flush_ssb(ssb_t& ssb);

    hashtable_t m_hashtable;
    ssb_map_t m_ssb_map;
    mutable mutex_t m_hashtable_mutex;
    mutable mutex_t m_ssb_mutex;
};

}}}

#endif //DIPLOMA_REMSET_HPP
