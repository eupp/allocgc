#ifndef DIPLOMA_GC_FACTORY_HPP
#define DIPLOMA_GC_FACTORY_HPP

#include <memory>

#include <libprecisegc/details/gc_strategy.hpp>
#include <libprecisegc/gc_options.hpp>

namespace precisegc { namespace details {

class gc_factory
{
public:
    static std::unique_ptr<gc_strategy> create(const gc_options& options);
};

}}

#endif //DIPLOMA_GC_FACTORY_HPP
