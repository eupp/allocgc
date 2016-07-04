#include <libprecisegc/details/collectors/packet_manager.hpp>

#include <libprecisegc/details/utils/make_unique.hpp>

namespace precisegc { namespace details { namespace collectors {

mark_packet::mark_packet()
    : m_size(0)
{}

void mark_packet::push(const managed_ptr& mp)
{
    assert(!is_full());
    m_data[m_size++] = mp;
}

managed_ptr mark_packet::pop()
{
    assert(m_size > 0);
    return m_data[--m_size];
}

bool mark_packet::is_full() const
{
    return m_size == SIZE;
}

bool mark_packet::is_almost_full() const
{
    return m_size >= SIZE / 2;
}

bool mark_packet::is_partial_full() const
{
    return m_size < SIZE / 2;
}

bool mark_packet::is_empty() const
{
    return m_size == 0;
}

void packet_manager::packet_pool::push(std::unique_ptr<mark_packet> packet)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_packets.push_back(std::move(packet));
}

std::unique_ptr<mark_packet> packet_manager::packet_pool::pop()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_packets.empty()) {
        return nullptr;
    }
    auto packet = std::move(m_packets.back());
    m_packets.pop_back();
    return packet;
}

size_t packet_manager::packet_pool::size() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_packets.size();
}

packet_manager::packet_manager()
    : m_packets_count(0)
{}

std::unique_ptr<mark_packet> packet_manager::pop_input_packet()
{
    auto input_packet = m_full_packets.pop();
    if (!input_packet) {
        input_packet = m_partial_packets.pop();
    }
    return input_packet;
}

std::unique_ptr<mark_packet> packet_manager::pop_output_packet()
{
    auto output_packet = m_empty_packets.pop();
    if (!output_packet) {
        output_packet = m_partial_packets.pop();
    }
    if (!output_packet) {
        m_packets_count.fetch_add(1, std::memory_order_acq_rel);
        output_packet = utils::make_unique<mark_packet>();
    }
    return output_packet;
}

void packet_manager::push_packet(std::unique_ptr<mark_packet> packet)
{
    if (packet->is_empty()) {
        m_empty_packets.push(std::move(packet));
    } else if (packet->is_partial_full()) {
        m_partial_packets.push(std::move(packet));
    } else if (packet->is_full()) {
        m_full_packets.push(std::move(packet));
    }
}

bool packet_manager::is_no_input() const
{
    return m_empty_packets.size() == m_packets_count.load(std::memory_order_acquire);
}

}}}

