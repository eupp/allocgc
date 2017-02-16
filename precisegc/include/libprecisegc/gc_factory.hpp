#ifndef DIPLOMA_GC_FACTORY_HPP
#define DIPLOMA_GC_FACTORY_HPP

#include <libprecisegc/gc_strategy.hpp>
#include <libprecisegc/gc_common.hpp>

namespace precisegc {

class gc_factory
{
public:
    struct options
    {
        size_t      heapsize            = std::numeric_limits<size_t>::max();
        size_t      threads_available   = std::thread::hardware_concurrency();
        bool        conservative        = false;
        bool        compacting          = false;
        bool        incremental         = false;
        bool        manual_init         = false;
        bool        print_stat          = false;
        gc_loglevel loglevel            = gc_loglevel::SILENT;
    };

    static std::unique_ptr<gc_strategy> create(const options& opt);
};

}

#endif //DIPLOMA_GC_FACTORY_HPP
