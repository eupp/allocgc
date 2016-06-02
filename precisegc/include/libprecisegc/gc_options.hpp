#ifndef DIPLOMA_GC_OPTIONS_HPP
#define DIPLOMA_GC_OPTIONS_HPP

namespace precisegc {

enum class gc_strategy
{
      SERIAL
    , INCREMENTAL
};

enum class gc_compacting
{
      ENABLED
    , DISABLED
};

struct gc_options
{
    gc_strategy     strategy;
    gc_compacting   compacting;
};

}

#endif //DIPLOMA_GC_OPTIONS_HPP
