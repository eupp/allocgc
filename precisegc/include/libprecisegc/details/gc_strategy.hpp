#ifndef DIPLOMA_GC_STRATEGY_HPP
#define DIPLOMA_GC_STRATEGY_HPP

#include <chrono>

#include <libprecisegc/gc_init_options.hpp>
#include <libprecisegc/details/gc_interface.hpp>
#include <libprecisegc/details/collectors/indexed_managed_object.hpp>
#include <libprecisegc/details/gc_handle.hpp>
#include <libprecisegc/details/utils/block_ptr.hpp>

namespace precisegc { namespace details {

class gc_strategy
{
public:
    virtual ~gc_strategy() {}

    virtual gc_pointer_type allocate(size_t size) = 0;
    virtual void new_cell(const indexed_managed_object& ptr) = 0;

    virtual byte* rbarrier(const gc_handle& handle) = 0;
    virtual void  wbarrier(gc_handle& dst, const gc_handle& src) = 0;

    virtual void interior_wbarrier(gc_handle& handle, ptrdiff_t offset) = 0;

    virtual gc_run_stats gc(const gc_options&) = 0;

    virtual gc_info info() const = 0;
};

}}

#endif //DIPLOMA_GC_STRATEGY_HPP
