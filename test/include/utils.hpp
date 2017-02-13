#ifndef DIPLOMA_UTILS_HPP
#define DIPLOMA_UTILS_HPP

#include <libprecisegc/gc_alloc.hpp>
#include <libprecisegc/details/gc_interface.hpp>
#include <libprecisegc/details/collectors/gc_new_stack_entry.hpp>

inline bool get_mark(const precisegc::gc_alloc::response& rsp)
{
    using namespace precisegc;
    using namespace precisegc::details;
    using namespace precisegc::details::allocators;

    collectors::gc_new_stack_entry* stack_entry = reinterpret_cast<collectors::gc_new_stack_entry*>(rsp.buffer());
    gc_memory_descriptor* descr = stack_entry->descriptor;
    return descr->get_mark(rsp.cell_start());
}

inline void set_mark(const precisegc::gc_alloc::response& rsp, bool mark)
{
    using namespace precisegc;
    using namespace precisegc::details;
    using namespace precisegc::details::allocators;

    collectors::gc_new_stack_entry* stack_entry = reinterpret_cast<collectors::gc_new_stack_entry*>(rsp.buffer());
    gc_memory_descriptor* descr = stack_entry->descriptor;
    descr->set_mark(rsp.cell_start(), mark);
}

inline bool get_pin(const precisegc::gc_alloc::response& rsp)
{
    using namespace precisegc;
    using namespace precisegc::details;
    using namespace precisegc::details::allocators;

    collectors::gc_new_stack_entry* stack_entry = reinterpret_cast<collectors::gc_new_stack_entry*>(rsp.buffer());
    gc_memory_descriptor* descr = stack_entry->descriptor;
    return descr->get_pin(rsp.cell_start());
}

inline void set_pin(const precisegc::gc_alloc::response& rsp, bool pin)
{
    using namespace precisegc;
    using namespace precisegc::details;
    using namespace precisegc::details::allocators;

    collectors::gc_new_stack_entry* stack_entry = reinterpret_cast<collectors::gc_new_stack_entry*>(rsp.buffer());
    gc_memory_descriptor* descr = stack_entry->descriptor;
    descr->set_pin(rsp.cell_start(), pin);
}

inline precisegc::details::gc_lifetime_tag get_lifetime_tag(const precisegc::gc_alloc::response& rsp)
{
    using namespace precisegc;
    using namespace precisegc::details;
    using namespace precisegc::details::allocators;

    collectors::gc_new_stack_entry* stack_entry = reinterpret_cast<collectors::gc_new_stack_entry*>(rsp.buffer());
    gc_memory_descriptor* descr = stack_entry->descriptor;
    return descr->get_lifetime_tag(rsp.cell_start());
}

inline void commit(const precisegc::gc_alloc::response& rsp)
{
    using namespace precisegc;
    using namespace precisegc::details;
    using namespace precisegc::details::allocators;

    collectors::gc_new_stack_entry* stack_entry = reinterpret_cast<collectors::gc_new_stack_entry*>(rsp.buffer());
    gc_memory_descriptor* descr = stack_entry->descriptor;
    descr->commit(rsp.cell_start());
}

inline void commit(const precisegc::gc_alloc::response& rsp, const precisegc::gc_type_meta* type_meta)
{
    using namespace precisegc;
    using namespace precisegc::details;
    using namespace precisegc::details::allocators;

    collectors::gc_new_stack_entry* stack_entry = reinterpret_cast<collectors::gc_new_stack_entry*>(rsp.buffer());
    gc_memory_descriptor* descr = stack_entry->descriptor;
    descr->commit(rsp.cell_start(), type_meta);
}

#endif //DIPLOMA_UTILS_HPP
