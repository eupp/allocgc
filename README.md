## Precise GC for C++

Precise garbage collection for C++ based on smart pointers. 
The project is still in development. Everything could change in the future

### Build

```bash
cd {PRJ_DIR}
cmake .
make libprecisegc
```

### Requirements

* C++11
* Boost 1.54 or greater
* GoogleTest (for unit-tests)

### Current limitations

* Linux-only (pthreads)
* x64 
* gcc 4.8 or greater (not tested with other compilers)

### Description

This is a precise garbage collector for C++. 
Like `std::shared_ptr` it uses smart pointers (called gc_ptr) to add automatic memory management into language, 
but our library implements tracing garbage collection instead of reference counting. 
Unlike famous Boehm-Demers-Weiser garbage collector our library doesn't scan thread's stacks conservatively
but maintains the root set dynamically during program execution. 

Currently the following features are supported:

* Pointers to arrays, pointer arithmetic with them
* Heap compactification
* Incremental marking (concurrently with the mutator)


### Usage

Two main primitives of the library are template class `gc_ptr` and template function `gc_new`. 
`gc_ptr` stores managed pointer (i.e. garbage collected pointer, unlike raw C pointer), 
`gc_new` allocates objects on managed heap. 
Before using any of library's primitives one should initialize garbage collector via `gc_init` function.

```C++
#include <libprecisegc/libprecisegc.hpp>

using namespace precisegc;

struct A {
    int x, y;
    gc_ptr<A> p;
    
    A(int x_, int y_) : x(x_), y(y_) {}
}

int main() {
    gc_init();
    
    gc_ptr<A> pA = gc_new<A>();
    a->x = 0;
    a->p = gc_new<A>(1, 0);
    
    return 0;
}
```

Pointers to arrays are also supported:

```C++
gc_ptr<A[]> pA = gc_new<A[]>(1000);
pA[10] = A(42, 0);
```

Sometimes it's necessary to get plain raw pointer to managed object. 
However, because collector can move objects, some additional efforts are need to be made to do that.
Library user should "pin" object in order to inform the collector that this object cannot be moved. 


```C++
gc_ptr<A> pA = gc_new<A>();
gc_pin<A> pin = pa.pin();
A* ptr = pin.get();
}
```

It is important that raw pointer should not outlive corresponding `gc_pin` object. 
Otherwise the collector may move the managed object and the raw pointer will dangle.

Similar rule applies to references.

```C++
gc_ref<A> rA = *pA;
A& ref = ra; 
}
```

Compare with following code:

```C++
A& ref = *pA;  // ERROR
}
```

### Tunning GC

Additional options could be passed to `gc_init`:

```C++
// Header file gc_init_options.hpp

struct gc_init_options
{
    size_t              heapsize;
    size_t              threads_available;
    gc_algo             algo;
    gc_initiation       initiation;
    gc_compacting       compacting;
    gc_loglevel         loglevel;
    bool                print_stat;
};

// main.cpp

int main() {
    gc_init_options ops;
    gc_init(ops);
    
    return 0;
}
```

* heapsize - upper limit for managed heap size in bytes
* threads_available - number of threads that collector could use internally
* algo - gc algorithm
    * gc_algo::SERIAL - serial stop-the-world collector
    * gc_algo::INCREMENTAL - collector with concurrent marking and stop-the-world sweeping/compacting
* initiation - policy of gc initiation
    * MANUAL - gc is launched only by user's request
    * SPACE_BASED - gc is launched when heap size exceeded
    * DEFAULT - default policy
* compacting - enables/disables heap compactification
    * gc_compacting::ENABLED 
    * gc_compacting::DISABLED
* loglevel - logging
    * DEBUG
    * INFO
    * WARNING
    * ERROR
    * SILENT
* print_stat - when true short statistic is printed to stderr