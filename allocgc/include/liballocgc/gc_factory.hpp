#ifndef ALLOCGC_GC_FACTORY_HPP
#define ALLOCGC_GC_FACTORY_HPP

#include <liballocgc/gc_strategy.hpp>
#include <liballocgc/gc_common.hpp>

namespace allocgc {

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

#endif //ALLOCGC_GC_FACTORY_HPP
