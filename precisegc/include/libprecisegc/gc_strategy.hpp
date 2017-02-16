#ifndef DIPLOMA_GC_STRATEGY_HPP
#define DIPLOMA_GC_STRATEGY_HPP

#include <chrono>

#include <libprecisegc/gc_alloc.hpp>
#include <libprecisegc/gc_handle.hpp>
#include <libprecisegc/details/gc_interface.hpp>
#include <libprecisegc/details/collectors/gc_new_stack_entry.hpp>

namespace precisegc {

class gc_strategy
{
public:
    virtual ~gc_strategy() {}

    virtual gc_alloc::response allocate(const gc_alloc::request& rqst) = 0;

    virtual void abort(const gc_alloc::response& rsp) = 0;
    virtual void commit(const gc_alloc::response& rsp) = 0;
    virtual void commit(const gc_alloc::response& rsp, const gc_type_meta* type_meta) = 0;

    virtual gc_offsets make_offsets(const gc_alloc::response& rsp) = 0;

    virtual void register_handle(gc_handle& handle, byte* ptr) = 0;
    virtual void deregister_handle(gc_handle& handle) = 0;

    virtual byte* register_pin(const gc_handle& handle) = 0;
    virtual void  deregister_pin(byte* pin) = 0;

    virtual byte* push_pin(const gc_handle& handle) = 0;
    virtual void  pop_pin(byte* pin) = 0;

    virtual byte* rbarrier(const gc_handle& handle) = 0;
    virtual void  wbarrier(gc_handle& dst, const gc_handle& src) = 0;

    virtual void interior_wbarrier(gc_handle& handle, ptrdiff_t offset) = 0;

    virtual void register_thread(const thread_descriptor& descr) = 0;
    virtual void deregister_thread(std::thread::id id) = 0;

    virtual gc_run_stat gc(const gc_options&) = 0;

    virtual gc_info info() const = 0;
};

}

#endif //DIPLOMA_GC_STRATEGY_HPP
