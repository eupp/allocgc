#ifndef DIPLOMA_FORWARDING_LIST_H
#define DIPLOMA_FORWARDING_LIST_H

#include <vector>

namespace precisegc { namespace details {

class forward_ptr_entry
{
public:

    forward_ptr_entry(void* from, void* to)
        : m_from(from)
        , m_to(to)
    {}

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
