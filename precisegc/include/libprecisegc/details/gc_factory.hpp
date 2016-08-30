#ifndef DIPLOMA_GC_FACTORY_HPP
#define DIPLOMA_GC_FACTORY_HPP

#include <memory>

#include <libprecisegc/details/gc_strategy.hpp>
#include <libprecisegc/details/initiation_policy.hpp>
#include <libprecisegc/gc_init_options.hpp>

namespace precisegc { namespace details {

class gc_factory
{
public:
    static std::unique_ptr<initiation_policy> create_initiation_policy(gc_strategy* gc,
                                                                       const gc_init_options& options);
    static std::unique_ptr<gc_strategy> create_gc(const gc_init_options& options);
private:
    static std::unique_ptr<initiation_policy> create_space_based_policy(gc_strategy* gc, size_t max_heap_size);
};

}}

#endif //DIPLOMA_GC_FACTORY_HPP
