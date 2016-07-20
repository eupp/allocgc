#ifndef DIPLOMA_GC_OPTIONS_HPP
#define DIPLOMA_GC_OPTIONS_HPP

#include <cstddef>
#include <limits>

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
    , OFF
};

struct gc_options
{
    size_t              heapsize    = std::numeric_limits<size_t>::max();
    gc_type             type        = gc_type::SERIAL;
    gc_init_strategy    init        = gc_init_strategy::DEFAULT;
    gc_compacting       compacting  = gc_compacting::DISABLED;
    gc_loglevel         loglevel    = gc_loglevel::OFF;
    bool                print_stat  = false;
};

}

#endif //DIPLOMA_GC_OPTIONS_HPP
