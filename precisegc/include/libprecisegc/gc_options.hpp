#ifndef DIPLOMA_GC_OPTIONS_HPP
#define DIPLOMA_GC_OPTIONS_HPP

#include <cstddef>
#include <limits>
#include <thread>

namespace precisegc {

enum class gc_type {
      SERIAL
    , INCREMENTAL
};

enum class gc_compacting {
      ENABLED
    , DISABLED
};

enum class gc_init_strategy {
      MANUAL
    , SPACE_BASED
    , DEFAULT
};

enum class gc_loglevel {
      DEBUG
    , INFO
    , WARNING
    , ERROR
    , SILENT
};

struct gc_options
{
    size_t              heapsize          = std::numeric_limits<size_t>::max();
    size_t              threads_available = std::thread::hardware_concurrency();
    gc_type             type              = gc_type::SERIAL;
    gc_init_strategy    init              = gc_init_strategy::DEFAULT;
    gc_compacting       compacting        = gc_compacting::DISABLED;
    gc_loglevel         loglevel          = gc_loglevel::SILENT;
    bool                print_stat        = false;
};

}

#endif //DIPLOMA_GC_OPTIONS_HPP
