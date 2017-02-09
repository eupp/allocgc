#ifndef DIPLOMA_LIST_FORWARDING_HPP
#define DIPLOMA_LIST_FORWARDING_HPP

#include <cstdlib>
#include <cstring>
#include <vector>
#include <algorithm>

#include <libprecisegc/gc_common.hpp>
#include <libprecisegc/details/gc_handle.hpp>

class test_forwarding
{
public:
    typedef precisegc::byte byte;

    struct entry
    {
        entry(byte* from_, byte* to_)
            : from(from_)
            , to(to_)
        {}

        byte* from;
        byte* to;
    };

    typedef std::vector<entry> containter_t;

    void create(byte* from, byte* to)
    {
//        memcpy(to, from, obj_size);
        m_frwd_list.emplace_back(from, to);
    }

    void forward(precisegc::details::gc_handle* handle) const
    {
        using namespace precisegc::details;

        byte* ptr = gc_handle_access::get<std::memory_order_relaxed>(*handle);
        auto it = std::find_if(m_frwd_list.begin(), m_frwd_list.end(), [ptr] (const entry& e) {
            return ptr == e.from;
        });
        if (it != m_frwd_list.end()) {
            gc_handle_access::set<std::memory_order_relaxed>(*handle, it->to);
        }
    }

    containter_t& get_forwarding_list()
    {
        return m_frwd_list;
    }
private:
    std::vector<entry> m_frwd_list;
};

#endif //DIPLOMA_LIST_FORWARDING_HPP
