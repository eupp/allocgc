#include <libprecisegc/details/collectors/remset.hpp>

namespace precisegc { namespace details { namespace collectors {

void remset::add(object_meta* meta)
{
    static thread_local ssb_t& ssb = get_ssb();
    if (ssb.is_full()) {
        flush_ssb(ssb);
    }
    ssb.push(meta);
}

object_meta* remset::get()
{
    std::lock_guard<mutex_t> lock(m_hashtable_mutex);
    assert(!m_hashtable.empty());
    auto it = m_hashtable.begin();
    object_meta* meta = *it;
    m_hashtable.erase(it);
    return meta;
}

bool remset::empty() const
{
    std::lock_guard<mutex_t> lock(m_hashtable_mutex);
    return m_hashtable.empty();
}

remset::const_iterator remset::begin() const
{
    return m_hashtable.begin();
}

remset::const_iterator remset::end() const
{
    return m_hashtable.end();
}

remset::ssb_t& remset::get_ssb()
{
    std::lock_guard<mutex_t> lock(m_ssb_mutex);
    return m_ssb_map[std::this_thread::get_id()];
}

void remset::flush_buffers()
{
    for (auto it = m_ssb_map.begin(); it != m_ssb_map.end(); ++it) {
        flush_ssb(it->second);
    }
}

void remset::flush_ssb(ssb_t& ssb)
{
    std::lock_guard<mutex_t> lock(m_hashtable_mutex);
    m_hashtable.insert(ssb.begin(), ssb.end());
    ssb.clear();
}

remset::sequential_store_buffer::sequential_store_buffer()
    : m_size(0)
{}

void remset::sequential_store_buffer::push(object_meta* meta)
{
    assert(!is_full());
    m_data[m_size++] = meta;
}

void remset::sequential_store_buffer::clear()
{
    m_size = 0;
}

bool remset::sequential_store_buffer::is_full() const
{
    return m_size == SSB_SIZE;
}

remset::sequential_store_buffer::const_iterator remset::sequential_store_buffer::begin() const
{
    return m_data;
}

remset::sequential_store_buffer::const_iterator remset::sequential_store_buffer::end() const
{
    return m_data + m_size;
}

}}}
