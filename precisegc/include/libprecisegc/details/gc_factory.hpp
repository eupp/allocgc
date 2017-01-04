#ifndef DIPLOMA_GC_FACTORY_HPP
#define DIPLOMA_GC_FACTORY_HPP

#include <memory>

#include <libprecisegc/details/gc_strategy.hpp>
#include <libprecisegc/gc_init_options.hpp>

namespace precisegc { namespace details {

class gc_factory
{
public:
    static std::unique_ptr<gc_strategy> create_gc(const gc_init_options& options);
};

}}

#endif //DIPLOMA_GC_FACTORY_HPP
