#ifndef DIPLOMA_GC_OPTIONS_HPP
#define DIPLOMA_GC_OPTIONS_HPP

namespace precisegc {

enum class gc_type
{
      SERIAL
    , INCREMENTAL
};

enum class gc_compacting
{
      ENABLED
    , DISABLED
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
    gc_type         type;
    gc_compacting   compacting;
    gc_loglevel     loglevel;
    bool            print_stat;
};

}

#endif //DIPLOMA_GC_OPTIONS_HPP
