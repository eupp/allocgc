#ifndef DIPLOMA_CLASS_META_H
#define DIPLOMA_CLASS_META_H

#include <memory>
#include <vector>
#include <pthread.h>

#include "mutex.h"

namespace precisegc { namespace details {

template <typename T>
class class_meta_provider;

class class_meta
{
public:
    const std::vector<size_t>& get_offsets() const noexcept
    {
        return m_offsets;
    }

    size_t get_type_size() const noexcept
    {
        return m_type_size;
    }

    template <typename T>
    friend class class_meta_provider;
private:
    class_meta(size_t type_size, const std::vector<size_t>& offsets)
        : m_offsets(offsets)
        , m_type_size(type_size)
    {}

    std::vector<size_t> m_offsets;
    size_t m_type_size;
};

template <typename T>
class class_meta_provider
{
public:

    static bool is_created()
    {
        lock_guard<recursive_mutex> lock(meta_mutex);
        return meta_inf != nullptr;
    }

    static void create_meta(const std::vector<size_t>& offsets)
    {
        lock_guard<recursive_mutex> lock(meta_mutex);
        if (is_created()) {
            return;
        }
        meta_inf.reset(new class_meta(sizeof(T), offsets));
    }

    static const class_meta& get_meta()
    {
        // we don't need syncronization here since class_meta is immutable object
//        assert(meta_inf);
        return *meta_inf;
    }

    static const class_meta* get_meta_ptr()
    {
        // we don't need syncronization here since class_meta is immutable object
//        assert(meta_inf);
        return meta_inf.get();
    }

private:
    static recursive_mutex meta_mutex;
    static std::unique_ptr<class_meta> meta_inf;
};

template <typename T>
recursive_mutex class_meta_provider<T>::meta_mutex;

template <typename T>
std::unique_ptr<class_meta> class_meta_provider<T>::meta_inf = nullptr;

} }

#endif //DIPLOMA_CLASS_META_H
