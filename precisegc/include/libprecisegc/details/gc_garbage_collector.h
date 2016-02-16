#ifndef DIPLOMA_GC_GARBAGE_COLLECTOR_H
#define DIPLOMA_GC_GARBAGE_COLLECTOR_H

namespace precisegc { namespace details {

class gc_garbage_collector
{
public:

    void start_marking();
    void wait_for_marking_finished();

private:

};

}}

#endif //DIPLOMA_GC_GARBAGE_COLLECTOR_H
