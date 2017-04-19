#ifndef ALLOCGC_GC_HOOKS_HPP
#define ALLOCGC_GC_HOOKS_HPP

#include <memory>

#include <liballocgc/gc_alloc.hpp>
#include <liballocgc/details/collectors/gc_new_stack_entry.hpp>
#include <liballocgc/details/gc_interface.hpp>

namespace allocgc { namespace details {

void gc_register_thread(const thread_descriptor& descr);
void gc_deregister_thread(std::thread::id id);

void gc_initiation_point(initiation_point_type ipoint, const gc_options& opt);

bool gc_increase_heap_size(size_t alloc_size);
void gc_decrease_heap_size(size_t size);
void gc_set_heap_limit(size_t size);
void gc_expand_heap();

gc_stat gc_get_stats();

}}

#endif //ALLOCGC_GC_HOOKS_HPP
