#ifndef DIPLOMA_GC_HOOKS_HPP
#define DIPLOMA_GC_HOOKS_HPP

#include <libprecisegc/details/managed_ptr.hpp>
#include <libprecisegc/details/gc_untyped_ptr.h>
#include <libprecisegc/details/logging.h>

namespace precisegc { namespace details {

enum class gc_strategy {
      SERIAL
    , INCREMENTAL
};

enum class gc_compacting {
      ENABLED
    , DISABLED
};

enum class gc_phase {
    IDLE,
    MARKING,
    COMPACTING
};

class gc_options
{
public:
    gc_options() = default;

    gc_strategy strategy() const
    {
        return m_strategy;
    }

    gc_compacting compacting() const
    {
        return m_compacting;
    }

    logging::loglevel loglevel() const
    {
        return m_loglevel;
    }

    gc_options& set_strategy(gc_strategy s)
    {
        m_strategy = s;
        return *this;
    }

    gc_options& set_compacting(gc_compacting cmpct)
    {
        m_compacting = cmpct;
    }

    gc_options& set_loglevel(logging::loglevel lv)
    {
        m_loglevel = lv;
    }
private:
    gc_strategy m_strategy;
    gc_compacting m_compacting;
    logging::loglevel m_loglevel;
};

void gc_init(const gc_options& opts);

managed_ptr gc_allocate(size_t size);

void gc_write_barrier(gc_untyped_ptr& dst, const gc_untyped_ptr& src);

gc_phase gc_get_phase();

byte* gc_load(const atomic_byte_ptr& p);
void  gc_store(atomic_byte_ptr& p, byte* value);

}}

#endif //DIPLOMA_GC_HOOKS_HPP
