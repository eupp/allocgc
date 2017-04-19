#ifndef ALLOCGC_LIST_FORWARDING_HPP
#define ALLOCGC_LIST_FORWARDING_HPP

#include <cstdlib>
#include <cstring>
#include <vector>
#include <algorithm>

#include <liballocgc/gc_common.hpp>

class test_forwarding
{
public:
    typedef allocgc::byte byte;

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

    void forward(allocgc::gc_handle* handle) const
    {
        using namespace allocgc;

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

#endif //ALLOCGC_LIST_FORWARDING_HPP
