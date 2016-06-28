#ifndef DIPLOMA_GC_STAT_HPP
#define DIPLOMA_GC_STAT_HPP

#include <libprecisegc/details/gc_clock.hpp>

namespace precisegc {

typedef details::gc_clock::duration gc_duration;

struct gc_stat
{
    size_t      gc_count;
    gc_duration gc_time;
};

}

#endif //DIPLOMA_GC_STAT_HPP
