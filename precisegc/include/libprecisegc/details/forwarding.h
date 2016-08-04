#ifndef DIPLOMA_FORWARDING_LIST_H
#define DIPLOMA_FORWARDING_LIST_H

#include <vector>
#include <cstring>
#include <unordered_map>

#include "object_meta.hpp"
#include "memory_descriptor.hpp"

namespace precisegc { namespace details {

class list_forwarding
{
public:
    struct entry
    {
        entry(void* from_, void* to_);
        void* from;
        void* to;
    };

    void create(void* from, void* to, size_t obj_size);
    void forward(void* ptr) const;

    std::vector<entry>& get_list();
private:
    std::vector<entry> m_frwd_list;
};

class intrusive_forwarding
{
public:
    intrusive_forwarding();
    void create(void* from, void* to, size_t obj_size);
    void forward(void* ptr) const;
    void join(const intrusive_forwarding& other);
};

}}

#endif //DIPLOMA_FORWARDING_LIST_H
