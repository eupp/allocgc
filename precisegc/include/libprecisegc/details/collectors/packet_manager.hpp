#ifndef DIPLOMA_PACKET_MANAGER_HPP
#define DIPLOMA_PACKET_MANAGER_HPP

#include <atomic>
#include <vector>
#include <memory>
#include <mutex>

#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/managed_ptr.hpp>

namespace precisegc { namespace details { namespace collectors {

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
private:
    static const size_t SIZE = 256;
    managed_ptr m_data[SIZE];
    size_t m_size;
};

class packet_manager : private utils::noncopyable, private utils::nonmovable
{
    class packet_pool : private utils::noncopyable, private utils::nonmovable
    {
    public:
        packet_pool() = default;

        void push(std::unique_ptr<mark_packet> packet);
        std::unique_ptr<mark_packet> pop();

        size_t size() const;
    private:
        std::vector<std::unique_ptr<mark_packet>> m_packets;
        std::mutex m_mutex;
    };
public:
    packet_manager();

    std::unique_ptr<mark_packet> pop_input_packet();
    std::unique_ptr<mark_packet> pop_output_packet();
    void push_packet(std::unique_ptr<mark_packet> packet);
    bool is_no_input() const;
private:
    packet_pool m_full_packets;
    packet_pool m_partial_packets;
    packet_pool m_empty_packets;
    std::atomic<size_t> m_packets_count;
};

}}}

#endif //DIPLOMA_PACKET_MANAGER_HPP
