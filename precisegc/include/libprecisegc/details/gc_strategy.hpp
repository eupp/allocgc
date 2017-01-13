#ifndef DIPLOMA_GC_STRATEGY_HPP
#define DIPLOMA_GC_STRATEGY_HPP

#include <chrono>

#include <libprecisegc/gc_init_options.hpp>
#include <libprecisegc/details/gc_cell.hpp>
#include <libprecisegc/details/gc_handle.hpp>
#include <libprecisegc/details/gc_interface.hpp>
#include <libprecisegc/details/allocators/gc_alloc_messaging.hpp>

namespace precisegc { namespace details {

class gc_strategy
{
public:
    virtual ~gc_strategy() {}

    virtual allocators::gc_alloc_response allocate(size_t obj_size, size_t obj_cnt, const gc_type_meta* tmeta) = 0;

    virtual void commit(gc_cell& cell) = 0;
    virtual void commit(gc_cell& cell, const gc_type_meta* type_meta) = 0;

    virtual byte* init_ptr(byte* ptr, bool root_flag) = 0;

    virtual byte* rbarrier(const gc_handle& handle) = 0;
    virtual void  wbarrier(gc_handle& dst, const gc_handle& src) = 0;

    virtual void interior_wbarrier(gc_handle& handle, ptrdiff_t offset) = 0;

    virtual gc_run_stats gc(const gc_options&) = 0;

    virtual gc_info info() const = 0;
};

}}

#endif //DIPLOMA_GC_STRATEGY_HPP
