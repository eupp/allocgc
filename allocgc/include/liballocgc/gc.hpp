#ifndef ALLOCGC_GC_H
#define ALLOCGC_GC_H

#include <thread>

#include <liballocgc/gc_strategy.hpp>
#include <liballocgc/gc_common.hpp>

namespace allocgc {

int gc_init(std::unique_ptr<gc_strategy> strategy);

void gc();

gc_stat gc_stats();

}

#endif //ALLOCGC_GC_H
