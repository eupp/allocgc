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
    return stack_entry->box_handle.get_mark();
}

inline void set_mark(const allocgc::gc_alloc::response& rsp, bool mark)
{
    using namespace allocgc;
    using namespace allocgc::details;
    using namespace allocgc::details::allocators;

    collectors::gc_new_stack_entry* stack_entry = reinterpret_cast<collectors::gc_new_stack_entry*>(rsp.buffer());
    stack_entry->box_handle.set_mark(mark);
}

inline bool get_pin(const allocgc::gc_alloc::response& rsp)
{
    using namespace allocgc;
    using namespace allocgc::details;
    using namespace allocgc::details::allocators;

    collectors::gc_new_stack_entry* stack_entry = reinterpret_cast<collectors::gc_new_stack_entry*>(rsp.buffer());
    return stack_entry->box_handle.get_pin();
}

inline void set_pin(const allocgc::gc_alloc::response& rsp, bool pin)
{
    using namespace allocgc;
    using namespace allocgc::details;
    using namespace allocgc::details::allocators;

    collectors::gc_new_stack_entry* stack_entry = reinterpret_cast<collectors::gc_new_stack_entry*>(rsp.buffer());
    stack_entry->box_handle.set_pin(pin);
}

inline allocgc::details::gc_lifetime_tag get_lifetime_tag(const allocgc::gc_alloc::response& rsp)
{
    using namespace allocgc;
    using namespace allocgc::details;
    using namespace allocgc::details::allocators;

    collectors::gc_new_stack_entry* stack_entry = reinterpret_cast<collectors::gc_new_stack_entry*>(rsp.buffer());
    return stack_entry->box_handle.get_lifetime_tag();
}

inline void commit(const allocgc::gc_alloc::response& rsp)
{
    using namespace allocgc;
    using namespace allocgc::details;
    using namespace allocgc::details::allocators;

    collectors::gc_new_stack_entry* stack_entry = reinterpret_cast<collectors::gc_new_stack_entry*>(rsp.buffer());
    stack_entry->box_handle.commit();
}

inline void commit(const allocgc::gc_alloc::response& rsp, const allocgc::gc_type_meta* type_meta)
{
    using namespace allocgc;
    using namespace allocgc::details;
    using namespace allocgc::details::allocators;

    collectors::gc_new_stack_entry* stack_entry = reinterpret_cast<collectors::gc_new_stack_entry*>(rsp.buffer());
    stack_entry->box_handle.commit(type_meta);
}

#endif //ALLOCGC_UTILS_HPP
