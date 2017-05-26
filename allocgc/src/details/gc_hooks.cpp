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

gc_stat gc_get_stats()
{
    return gc_serial_facade::stats();
}

}

}