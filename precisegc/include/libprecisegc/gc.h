#ifndef DIPLOMA_GC_H
#define DIPLOMA_GC_H

#include <libprecisegc/details/gc_hooks.hpp>

namespace precisegc {

inline void gc()
{
    details::gc_initation_point(details::initation_point_type::USER_REQUEST);
}

}

#endif //DIPLOMA_GC_H
