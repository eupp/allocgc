#ifndef DIPLOMA_GC_H
#define DIPLOMA_GC_H

#include <libprecisegc/details/garbage_collector.hpp>

namespace precisegc {

inline void gc()
{
    using namespace details;
    gci().initation_point(details::initation_point_type::USER_REQUEST);
}

}

#endif //DIPLOMA_GC_H
