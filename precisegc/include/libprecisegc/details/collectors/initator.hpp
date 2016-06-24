#ifndef DIPLOMA_GC_INITATOR_H
#define DIPLOMA_GC_INITATOR_H

#include <atomic>
#include <limits>
#include <list>
#include <memory>
#include <mutex>

#include <libprecisegc/details/gc_strategy.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/collectors/initation_policy.hpp>

namespace precisegc { namespace details { namespace collectors {

class initator : private utils::noncopyable, private utils::nonmovable
{
public:
    initator(serial_gc_strategy* gc, std::unique_ptr<initation_policy>&& policy);

    void initation_point(initation_point_type ipoint);

    initation_policy* get_policy() const;
private:
    serial_gc_strategy* m_gc;
    std::unique_ptr<initation_policy> m_policy;
    std::mutex m_mutex;
};

class incremental_initator : private utils::noncopyable, private utils::nonmovable
{
public:
    incremental_initator(incremental_gc_strategy* gc, std::unique_ptr<incremental_initation_policy>&& policy);

    void initation_point(initation_point_type ipoint);

    incremental_initation_policy* get_policy() const;
private:
    incremental_gc_strategy* m_gc;
    std::unique_ptr<incremental_initation_policy> m_policy;
    std::mutex m_mutex;
};

}}}

#endif //DIPLOMA_GC_INITATOR_H
