#ifndef DIPLOMA_FORWARDING_LIST_H
#define DIPLOMA_FORWARDING_LIST_H

#include <vector>
#include <cstring>

namespace precisegc { namespace details {

class list_forwarding
{
public:
    void create(void* from, void* to, size_t obj_size);
    void forward(void* ptr);
private:
    struct entry
    {
        entry(void* from_, void* to_);
        void* from;
        void* to;
    };

    std::vector<entry> m_frwd_list;
};

class intrusive_forwarding
{
public:
    void create(void* from, void* to, size_t obj_size);
    void forward(void* ptr);
};

}}

#endif //DIPLOMA_FORWARDING_LIST_H
