#ifndef DIPLOMA_GC_H
#define DIPLOMA_GC_H

#include <libprecisegc/details/gc_hooks.hpp>

namespace precisegc {

inline void gc()
{
    details::gc();
}

}

#endif //DIPLOMA_GC_H
