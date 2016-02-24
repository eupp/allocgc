#include "forwarding.h"

#include <cassert>

#include "object_meta.h"

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

void list_forwarding::forward(void* ptr)
{
    void*& from = * ((void**) ptr);
    for (auto& frwd: m_frwd_list) {
        if (from == frwd.from) {
            from = frwd.to;
        }
    }
}

void intrusive_forwarding::create(void* from, void* to, size_t obj_size)
{
    move_cell(from, to, obj_size);
    object_meta* meta = object_meta::get_meta_ptr(from, obj_size);
    meta->set_object_ptr(to);
}

void intrusive_forwarding::forward(void* ptr)
{
    object_meta* meta = object_meta::get_meta_ptr(from, obj_size);
}

}}