#ifndef DIPLOMA_FORWARDING_LIST_H
#define DIPLOMA_FORWARDING_LIST_H

#include <vector>
#include <cstring>

namespace precisegc { namespace details {

class forward_ptr_entry
{
public:

    forward_ptr_entry(void* from, void* to, size_t size)
        : m_from(from)
        , m_to(to)
    {
        if (from && to) {
            memcpy(to, from, size);
        }
    }

    void* from() const noexcept
    {
        return m_from;
    }

    void* to() const noexcept
    {
        return m_to;
    }

private:
    void* m_from;
    void* m_to;
};

typedef std::vector<forward_ptr_entry> forwarding_list;

}}

#endif //DIPLOMA_FORWARDING_LIST_H
