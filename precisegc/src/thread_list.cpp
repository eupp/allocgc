#include "thread_list.h"

#include <algorithm>

namespace precisegc { namespace details {

mutex thread_list::locked_instance::tl_mutex = mutex();

bool thread_list::empty() const
{
    return m_list.empty();
}

size_t thread_list::size() const
{
    return m_list.size();
}

thread_list::iterator thread_list::insert(const thread_handler& th)
{
    return m_list.insert(m_list.end(), th);
}

thread_list::iterator thread_list::remove(thread_list::iterator it)
{
    return m_list.erase(it.m_iterator);
}

thread_list::iterator thread_list::find(pthread_t th)
{
    return std::find_if(m_list.begin(), m_list.end(),
                        [&th](const thread_handler& other) {
                            return pthread_equal(other.thread, th);
                        });
}

thread_list::iterator thread_list::begin()
{
    return thread_list::iterator(m_list.begin());
}

thread_list::iterator thread_list::end()
{
    return thread_list::iterator(m_list.end());
}

thread_list::iterator::iterator(std::list<precisegc::thread_handler>::iterator it)
    : m_iterator(it)
{}

thread_handler& thread_list::iterator::operator*()
{
    return *m_iterator;
}

bool thread_list::iterator::equal(const thread_list::iterator& other) const noexcept
{
    return m_iterator == other.m_iterator;
}

void thread_list::iterator::increment() noexcept
{
    ++m_iterator;
}

void thread_list::iterator::decrement() noexcept
{
    --m_iterator;
}

}}