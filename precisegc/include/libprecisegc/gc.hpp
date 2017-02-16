#ifndef DIPLOMA_GC_H
#define DIPLOMA_GC_H

#include <thread>

#include <libprecisegc/gc_strategy.hpp>
#include <libprecisegc/gc_common.hpp>

namespace precisegc {

int gc_init(std::unique_ptr<gc_strategy> strategy);

void gc();

gc_stat gc_stats();

}

#endif //DIPLOMA_GC_H
