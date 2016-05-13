#ifndef DIPLOMA_MARKER_HPP
#define DIPLOMA_MARKER_HPP

#include <vector>
#include <mutex>

namespace precisegc { namespace details {

class marker
{
public:
    marker() = default;

    void trace_roots();

    void start_marking();
    void pause_marking();

    void wait_for_marking();

    size_t workers_num() const;
    size_t markers_num() const;

    void add_worker();
    void remove_worker();
private:

};

}}

#endif //DIPLOMA_MARKER_HPP
