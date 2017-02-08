#ifndef DIPLOMA_GC_HOOKS_HPP
#define DIPLOMA_GC_HOOKS_HPP

#include <memory>

#include <libprecisegc/gc_new_stack_entry.hpp>
#include <libprecisegc/details/gc_interface.hpp>
#include <libprecisegc/details/gc_strategy.hpp>
#include <libprecisegc/gc_stat.hpp>

namespace precisegc { namespace details {

void gc_initialize(std::unique_ptr<gc_strategy> strategy);

//byte* gc_init_ptr(byte* ptr, bool root_flag);

void gc_register_root(gc_handle* root);
void gc_deregister_root(gc_handle* root);

bool gc_is_root(const gc_handle& ptr);
bool gc_is_heap_ptr(const gc_handle* ptr);

allocators::gc_alloc_response gc_allocate(size_t obj_size, size_t obj_cnt, const gc_type_meta* tmeta);

void gc_commit(gc_cell& cell);
void gc_commit(gc_cell& cell, const gc_type_meta* type_meta);

void gc_register_handle(gc_handle& handle, byte* ptr);
void gc_deregister_handle(gc_handle& handle);

void gc_register_stack_entry(gc_new_stack_entry* stack_entry);
void gc_deregister_stack_entry(gc_new_stack_entry* stack_entry);

void gc_register_thread(std::thread::id id, byte* stack_start_addr);
void gc_deregister_thread(std::thread::id id);

void gc_initiation_point(initiation_point_type ipoint, const gc_options& opt);

void gc_add_to_index(const byte* mem, size_t size, gc_memory_descriptor* entry);
void gc_remove_from_index(const byte* mem, size_t size);

gc_memory_descriptor* gc_index_memory(const byte* mem);
gc_cell gc_index_object(byte* obj_start);

gc_info  gc_get_info();
gc_stat  gc_get_stats();

void gc_enable_print_stats();
void gc_disable_print_stats();

void gc_register_page(const byte* page, size_t size);
void gc_deregister_page(const byte* page, size_t size);

}}

#endif //DIPLOMA_GC_HOOKS_HPP
