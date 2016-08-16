#ifndef DIPLOMA_PACKET_MANAGER_HPP
#define DIPLOMA_PACKET_MANAGER_HPP

#include <atomic>
#include <array>
#include <memory>
#include <mutex>

#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/managed_ptr.hpp>

namespace precisegc { namespace details { namespace collectors {

class packet_manager;

class mark_packet : private utils::noncopyable, private utils::nonmovable
{
public:
    mark_packet();

    void push(const managed_ptr& mp);
    managed_ptr pop();

    bool is_full() const;
    bool is_almost_full() const;
    bool is_partial_full() const;
    bool is_empty() const;

    friend class packet_manager;
private:
    static const size_t SIZE = 256;
    managed_ptr m_data[SIZE];
    size_t m_size;
    mark_packet* m_next;
};

class packet_manager : private utils::noncopyable, private utils::nonmovable
{
    static const size_t PACKETS_COUNT = 256;

    class packet_pool : private utils::noncopyable, private utils::nonmovable
    {
    public:
        packet_pool();

        void push(mark_packet_handle packet);
        mark_packet_handle pop();

        size_t size() const;
    private:
        mark_packet* m_head;
        size_t m_size;
        mutable std::mutex m_mutex;
    };
public:
    typedef mark_packet* mark_packet_handle;

    packet_manager();

    mark_packet_handle pop_input_packet();
    mark_packet_handle pop_output_packet();
    void push_packet(mark_packet_handle packet);
    bool is_no_input() const;
private:
    std::array<mark_packet, PACKETS_COUNT> m_packets_storage;
    packet_pool m_full_packets;
    packet_pool m_partial_packets;
    packet_pool m_empty_packets;
    std::atomic<size_t> m_packets_count;
};

}}}

#endif //DIPLOMA_PACKET_MANAGER_HPP
