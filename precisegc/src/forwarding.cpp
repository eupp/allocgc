#include "forwarding.h"

#include <cassert>

#include "libprecisegc/details/ptrs/gc_untyped_ptr.hpp"
#include "managed_ptr.hpp"
#include "object_meta.hpp"

#include "logging.hpp"
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
{}

void intrusive_forwarding::create(void* from, void* to, size_t obj_size)
{
    logging::debug() << "create forwarding: from " << from << " to " << to;

    object_meta* meta = object_meta::get_meta_ptr(from, obj_size);
    meta->set_forward_pointer((byte*) to);
    move_cell(from, to, obj_size);
}

void intrusive_forwarding::forward(void* ptr) const
{
    ptrs::gc_untyped_ptr* gcptr = reinterpret_cast<ptrs::gc_untyped_ptr*>(ptr);
    void* from = gcptr->get();
    if (from) {
        try {
            auto cell_ptr = managed_ptr(reinterpret_cast<byte*>(from));
            object_meta* meta = cell_ptr.get_meta();
            if (meta->is_forwarded()) {
                void* to = meta->forward_pointer();
                logging::debug() << "fix ptr: from " << from << " to " << to;
                gcptr->forward(to);
            }
        } catch (unindexed_memory_exception& exc) {
            logging::error() << "Unindexed memory discovered: " << from;
            throw;
        }
    }
}

//void intrusive_forwarding::join(const intrusive_forwarding& other)
//{
//    return;
//}

}}