#include <liballocgc/details/gc_hooks.hpp>

#include <liballocgc/details/gc_facade.hpp>

#include <liballocgc/details/collectors/gc_serial.hpp>
#include <liballocgc/details/allocators/memory_index.hpp>

namespace allocgc {

namespace details {

typedef gc_facade<collectors::gc_serial> gc_serial_facade;

void gc_register_thread(const thread_descriptor& descr)
{
    gc_serial_facade::register_thread(descr);
}

void gc_deregister_thread(std::thread::id id)
{
    gc_serial_facade::deregister_thread(id);
}

void gc_initiation_point(initiation_point_type ipoint, const gc_options& opt)
{
    gc_serial_facade::initiation_point(ipoint, opt);
}

bool gc_increase_heap_size(size_t alloc_size)
{
    return gc_serial_facade::increment_heap_size(alloc_size);
}

void gc_decrease_heap_size(size_t size)
{
    gc_serial_facade::decrement_heap_size(size);
}

void gc_set_heap_limit(size_t size)
{
    gc_serial_facade::set_heap_limit(size);
}

void gc_expand_heap()
{
    gc_serial_facade::expand_heap();
}

gc_stat gc_get_stats()
{
    return gc_serial_facade::stats();
}

}

}