#ifndef DIPLOMA_FORWARDING_LIST_H
#define DIPLOMA_FORWARDING_LIST_H

#include <vector>
#include <cstring>
#include <unordered_map>

#include "object_meta.h"
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
private:
    template<typename T>
    struct fast_pointer_hash {
        size_t operator() (const T* val) const
        {
            uintptr_t ad = (uintptr_t)val;
            return (size_t)((13 * ad) ^ (ad >> 15));
//            return (size_t)(val) >> 3;
//            uintptr_t ad = (uintptr_t) val;
//            return (size_t)(ad ^ (ad >> 16));
        }
    };

    static const size_t CACHE_SIZE = 1024;
    typedef std::unordered_map<void*, memory_descriptor*, fast_pointer_hash<void>> cache_t;

    size_t m_frwd_cnt;
    mutable cache_t m_cache;
};

}}

#endif //DIPLOMA_FORWARDING_LIST_H
