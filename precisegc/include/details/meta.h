#ifndef DIPLOMA_META_H
#define DIPLOMA_META_H

#include <memory>
#include <vector>
#include <pthread.h>

#include "mutex.h"
#include "os.h"

namespace precisegc { namespace details {

template <typename T>
class meta_provider;

class meta
{
public:
    const std::vector<size_t>& get_offsets() const noexcept
    {
        return m_offsets;
    }

    template <typename T>
    friend class meta_provider;
private:
    meta(const std::vector<size_t>& offsets)
        : m_offsets(offsets)
    {}

    std::vector<size_t> m_offsets;
};

template <typename T>
class meta_provider
{
public:

    bool is_created()
    {
        mutex_lock<recursive_mutex> lock(&meta_mutex);
        return meta_inf != nullptr;
    }

    void create_meta(const std::vector<size_t>& offsets)
    {
        mutex_lock<recursive_mutex> lock(&meta_mutex);
        if (is_created()) {
            return;
        }
        meta_inf.reset(new meta(offsets));
    }

    const meta& get_meta()
    {
        // we don't need syncronization here since meta is immutable object
        assert(this->meta_inf);
        return *meta_inf;
    }

private:
    static recursive_mutex meta_mutex;
    static std::unique_ptr<meta> meta_inf;
};

template <typename T>
recursive_mutex meta_provider<T>::meta_mutex = recursive_mutex();

template <typename T>
std::unique_ptr<meta> meta_provider<T>::meta_inf = nullptr;

} }

#endif //DIPLOMA_META_H
