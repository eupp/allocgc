#ifndef ALLOCGC_UTILS_HPP
#define ALLOCGC_UTILS_HPP

#include <liballocgc/gc_alloc.hpp>
#include <liballocgc/details/gc_interface.hpp>
#include <liballocgc/details/collectors/gc_new_stack_entry.hpp>

inline bool get_mark(const allocgc::gc_alloc::response& rsp)
{
    using namespace allocgc;
    using namespace allocgc::details;
    using namespace allocgc::details::allocators;

    collectors::gc_new_stack_entry* stack_entry = reinterpret_cast<collectors::gc_new_stack_entry*>(rsp.buffer());
    gc_memory_descriptor* descr = stack_entry->descriptor;
    return descr->get_mark(rsp.cell_start());
}

inline void set_mark(const allocgc::gc_alloc::response& rsp, bool mark)
{
    using namespace allocgc;
    using namespace allocgc::details;
    using namespace allocgc::details::allocators;

    collectors::gc_new_stack_entry* stack_entry = reinterpret_cast<collectors::gc_new_stack_entry*>(rsp.buffer());
    gc_memory_descriptor* descr = stack_entry->descriptor;
    descr->set_mark(rsp.cell_start(), mark);
}

inline bool get_pin(const allocgc::gc_alloc::response& rsp)
{
    using namespace allocgc;
    using namespace allocgc::details;
    using namespace allocgc::details::allocators;

    collectors::gc_new_stack_entry* stack_entry = reinterpret_cast<collectors::gc_new_stack_entry*>(rsp.buffer());
    gc_memory_descriptor* descr = stack_entry->descriptor;
    return descr->get_pin(rsp.cell_start());
}

inline void set_pin(const allocgc::gc_alloc::response& rsp, bool pin)
{
    using namespace allocgc;
    using namespace allocgc::details;
    using namespace allocgc::details::allocators;

    collectors::gc_new_stack_entry* stack_entry = reinterpret_cast<collectors::gc_new_stack_entry*>(rsp.buffer());
    gc_memory_descriptor* descr = stack_entry->descriptor;
    descr->set_pin(rsp.cell_start(), pin);
}

inline allocgc::details::gc_lifetime_tag get_lifetime_tag(const allocgc::gc_alloc::response& rsp)
{
    using namespace allocgc;
    using namespace allocgc::details;
    using namespace allocgc::details::allocators;

    collectors::gc_new_stack_entry* stack_entry = reinterpret_cast<collectors::gc_new_stack_entry*>(rsp.buffer());
    gc_memory_descriptor* descr = stack_entry->descriptor;
    return descr->get_lifetime_tag(rsp.cell_start());
}

inline void commit(const allocgc::gc_alloc::response& rsp)
{
    using namespace allocgc;
    using namespace allocgc::details;
    using namespace allocgc::details::allocators;

    collectors::gc_new_stack_entry* stack_entry = reinterpret_cast<collectors::gc_new_stack_entry*>(rsp.buffer());
    gc_memory_descriptor* descr = stack_entry->descriptor;
    descr->commit(rsp.cell_start());
}

inline void commit(const allocgc::gc_alloc::response& rsp, const allocgc::gc_type_meta* type_meta)
{
    using namespace allocgc;
    using namespace allocgc::details;
    using namespace allocgc::details::allocators;

    collectors::gc_new_stack_entry* stack_entry = reinterpret_cast<collectors::gc_new_stack_entry*>(rsp.buffer());
    gc_memory_descriptor* descr = stack_entry->descriptor;
    descr->commit(rsp.cell_start(), type_meta);
}

#endif //ALLOCGC_UTILS_HPP
