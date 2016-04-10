#include "forwarding.h"

#include <cassert>

#include "gc_untyped_ptr.h"
#include "managed_ptr.h"
#include "object_meta.h"

#include "logging.h"
#include "gc_mark.h"

namespace precisegc { namespace details {

static void move_cell(void* from, void* to, size_t cell_size)
{
    assert(from);
    assert(to);
    memcpy(to, from, cell_size);
}

list_forwarding::entry::entry(void* from_, void* to_)
    : from(from_)
    , to(to_)
{}

void list_forwarding::create(void* from, void* to, size_t obj_size)
{
    move_cell(from, to, obj_size);
    m_frwd_list.emplace_back(from, to);
}

void list_forwarding::forward(void* ptr) const
{
    void*& from = * ((void**) ptr);
    for (auto& frwd: m_frwd_list) {
        if (from == frwd.from) {
            logging::debug() << "fix ptr: from " << from << " to " << frwd.to;
            from = frwd.to;
        }
    }
}

std::vector<list_forwarding::entry>& list_forwarding::get_list()
{
    return m_frwd_list;
}

intrusive_forwarding::intrusive_forwarding()
    : m_frwd_cnt(0)
{
    m_cache.reserve(2 * CACHE_SIZE);
}

void intrusive_forwarding::create(void* from, void* to, size_t obj_size)
{
    object_meta* meta = object_meta::get_meta_ptr(from, obj_size);
    meta->set_object_ptr(to);
    move_cell(from, to, obj_size);
    m_frwd_cnt++;
}

void intrusive_forwarding::forward(void* ptr) const
{
    gc_untyped_ptr* gcptr = reinterpret_cast<gc_untyped_ptr*>(ptr);
    void* from = gcptr->get();
    if (from) {
        try {
            object_meta* meta = nullptr;
            if (m_cache.count(from)) {
                meta = m_cache[from];
            } else {
                meta = get_object_header(from);
                if (m_cache.size() < CACHE_SIZE) {
                    m_cache[from] = meta;
                }
            }
            if (meta) {
                void* to = meta->get_object_ptr();
                if (from != to) {
                    logging::debug() << "fix ptr: from " << from << " to " << to;
                    gcptr->set(to);
                }
            }
        } catch (managed_cell_ptr::unindexed_memory_exception& exc) {
            logging::error() << "Unindexed memory discovered: " << from;
            throw;
        }
    }
}
}}