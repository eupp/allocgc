#ifndef DIPLOMA_GC_H
#define DIPLOMA_GC_H

#include <thread>

#include <libprecisegc/gc_init_options.hpp>
#include <libprecisegc/gc_stat.hpp>

namespace precisegc {

int gc_init(gc_init_options& options);

gc_stat gc_stats();

void gc();

}

#endif //DIPLOMA_GC_H
