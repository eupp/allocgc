#ifndef DIPLOMA_GC_HOOKS_HPP
#define DIPLOMA_GC_HOOKS_HPP

#include <memory>

#include <libprecisegc/gc_alloc.hpp>
#include <libprecisegc/details/collectors/gc_new_stack_entry.hpp>
#include <libprecisegc/details/gc_interface.hpp>
#include <libprecisegc/details/gc_strategy.hpp>
#include <libprecisegc/gc_stat.hpp>

namespace precisegc { namespace details {

void gc_initialize(std::unique_ptr<gc_strategy> strategy);

bool gc_is_heap_ptr(const gc_handle* ptr);

gc_alloc::response gc_allocate(const gc_alloc::request& rqst);

void gc_abort(const gc_alloc::response& rsp);
void gc_commit(const gc_alloc::response& rsp);
void gc_commit(const gc_alloc::response& rsp, const gc_type_meta* type_meta);

gc_offsets gc_make_offsets(const gc_alloc::response& rsp);

void gc_register_handle(gc_handle& handle, byte* ptr);
void gc_deregister_handle(gc_handle& handle);

void gc_register_thread(const thread_descriptor& descr);
void gc_deregister_thread(std::thread::id id);

void gc_initiation_point(initiation_point_type ipoint, const gc_options& opt);

bool gc_increase_heap_size(size_t alloc_size);
void gc_decrease_heap_size(size_t size);
void gc_set_heap_limit(size_t size);
void gc_expand_heap();

void gc_add_to_index(const byte* mem, size_t size, gc_memory_descriptor* entry);
void gc_remove_from_index(const byte* mem, size_t size);

gc_memory_descriptor* gc_index_memory(const byte* mem);
gc_cell gc_index_object(byte* obj_start);

gc_stat  gc_get_stats();

void gc_enable_print_stats();
void gc_disable_print_stats();

void gc_register_page(const byte* page, size_t size);
void gc_deregister_page(const byte* page, size_t size);

}}

#endif //DIPLOMA_GC_HOOKS_HPP
