#ifndef DIPLOMA_THREAD_LIST_H
#define DIPLOMA_THREAD_LIST_H

#include <list>
#include <iterator>
#include <pthread.h>

#include "../thread.h"
#include "mutex.h"
#include "iterator_base.h"
#include "iterator_access.h"

namespace precisegc { namespace details {

class thread_list
{
    typedef std::list<thread_handler> th_list_t;
public:

    static mutex instance_mutex;

    static thread_list& instance()
    {
        static thread_list list;
        return list;
    }

    class iterator;

    bool empty() const;
    size_t size() const;

    iterator find(pthread_t th);

    iterator insert(const thread_handler& th);
    iterator remove(iterator it);

    template <typename... Args>
    void emplace(Args... args)
    {
        m_list.emplace_back(std::forward<Args>(args)...);
    }

    iterator begin();
    iterator end();

    class iterator: public iterator_base<iterator, std::bidirectional_iterator_tag, thread_handler>
    {
    public:
        iterator(const iterator&) noexcept = default;
        iterator(iterator&&) noexcept = default;
        iterator& operator=(const iterator&) noexcept = default;
        iterator& operator=(iterator&&) noexcept = default;

        thread_handler& operator*();
        th_list_t::iterator operator->();

        friend class thread_list;
        friend class iterator_access<iterator>;
    private:
        iterator(th_list_t::iterator it);

        bool equal(const iterator& other) const noexcept;
        void increment() noexcept;
        void decrement() noexcept;

        th_list_t::iterator m_iterator;
    };

private:
    thread_list() = default;
    thread_list(const thread_list&) = delete;
    thread_list(thread_list&&) = delete;

    thread_list& operator=(const thread_list&) = delete;
    thread_list& operator=(thread_list&&) = delete;

    th_list_t m_list;
};

}}

#endif //DIPLOMA_THREAD_LIST_H
