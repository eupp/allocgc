#ifndef DIPLOMA_GC_H
#define DIPLOMA_GC_H

#include <libprecisegc/gc_options.hpp>
#include <libprecisegc/gc_stat.hpp>

namespace precisegc {

int gc_init(const gc_options& options = gc_options());

gc_stat gc_stats();

void gc();


}

#endif //DIPLOMA_GC_H
