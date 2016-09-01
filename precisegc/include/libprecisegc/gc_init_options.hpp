#ifndef DIPLOMA_GC_OPTIONS_HPP
#define DIPLOMA_GC_OPTIONS_HPP

#include <cstddef>
#include <limits>
#include <thread>

namespace precisegc {

enum class gc_algo {
      SERIAL
    , INCREMENTAL
};

enum class gc_compacting {
      ENABLED
    , DISABLED
};

enum class gc_initiation {
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

struct gc_init_options
{
    size_t              heapsize          = std::numeric_limits<size_t>::max();
    size_t              threads_available = std::thread::hardware_concurrency();
    gc_algo             algo              = gc_algo::SERIAL;
    gc_initiation       initiation        = gc_initiation::DEFAULT;
    gc_compacting       compacting        = gc_compacting::DISABLED;
    gc_loglevel         loglevel          = gc_loglevel::SILENT;
    bool                print_stat        = false;
};

}

#endif //DIPLOMA_GC_OPTIONS_HPP
