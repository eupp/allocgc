#ifndef DIPLOMA_REMSET_HPP
#define DIPLOMA_REMSET_HPP

#include <unordered_map>
#include <unordered_set>
#include <mutex>

#include <libprecisegc/details/object_meta.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details { namespace collectors {

class remset : private utils::noncopyable, private utils::nonmovable
{
    typedef std::unordered_set<object_meta*> hashtable_t;
public:
    typedef hashtable_t::const_iterator const_iterator;

    remset() = default;

    void add(object_meta* meta);
    object_meta* get();

    void flush_buffers();

    bool empty() const;

    const_iterator begin() const;
    const_iterator end() const;
private:
    class sequential_store_buffer : private utils::noncopyable, private utils::nonmovable
    {
    public:
        typedef object_meta* const* const_iterator;

        sequential_store_buffer();

        void push(object_meta* meta);
        void clear();

        bool is_full() const;

        const_iterator begin() const;
        const_iterator end() const;
    private:
        friend class remset;

        static const size_t SSB_SIZE = 64;

        object_meta* m_data[SSB_SIZE];
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
