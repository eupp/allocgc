#include <libprecisegc/details/gc_hooks.hpp>

#include <cassert>
#include <memory>

#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/recorder.hpp>

namespace precisegc { namespace details {

class garbage_collector : private utils::noncopyable, private utils::nonmovable
{
public:
    garbage_collector() = default;

    gc_interface* get_strategy() const
    {
        return m_strategy.get();
    }

    void set_strategy(std::unique_ptr<gc_interface> strategy)
    {
        m_strategy = std::move(strategy);
    }

    std::unique_ptr<gc_interface> reset_strategy(std::unique_ptr<gc_interface> strategy)
    {
        strategy.swap(m_strategy);
        return std::move(strategy);
    }

    managed_ptr allocate(size_t size)
    {
        assert(m_strategy);
        m_recorder.register_alloc_request();
        managed_ptr p = m_strategy->allocate(size);
        m_recorder.register_allocation(p.cell_size());
        return p;
    }

    byte* rbarrier(const atomic_byte_ptr& p)
    {
        assert(m_strategy);
        return m_strategy->rbarrier(p);
    }

    void  wbarrier(atomic_byte_ptr& dst, const atomic_byte_ptr& src)
    {
        assert(m_strategy);
        m_strategy->wbarrier(dst, src);
    }

    void initation_point(initation_point_type ipoint)
    {
        assert(m_strategy);
        m_strategy->initation_point(ipoint);
    }

    bool printer_enabled() const;
    void set_printer_enabled(bool enabled);

    void register_gc(const gc_sweep_stat& sweep_stat, const gc_pause_stat& pause_stat);
    void register_pause(const gc_pause_stat& pause_stat);
private:
    std::unique_ptr<gc_interface> m_strategy;
    recorder m_recorder;

};

static garbage_collector gc_instance;

gc_interface* gc_get_strategy()
{
    return gc_instance.get_strategy();
}

void gc_set_strategy(std::unique_ptr<gc_interface> strategy)
{
    gc_instance.set_strategy(std::move(strategy));
}

std::unique_ptr<gc_interface> gc_reset_strategy(std::unique_ptr<gc_interface> strategy)
{
    return gc_instance.reset_strategy(std::move(strategy));
}

void gc()
{
    gc_instance.initation_point(initation_point_type::USER_REQUEST);
};

managed_ptr gc_allocate(size_t size)
{
    return gc_instance.allocate(size);
}

byte* gc_rbarrier(const atomic_byte_ptr& p)
{
    return gc_instance.rbarrier(p);
}

void gc_wbarrier(atomic_byte_ptr& dst, const atomic_byte_ptr& src)
{
    gc_instance.wbarrier(dst, src);
}

void gc_initation_point(initation_point_type ipoint)
{
    gc_instance.initation_point(ipoint);
}

void register_gc(const gc_sweep_stat& sweep_stat, const gc_pause_stat& pause_stat)
{
    gc_instance.register_gc(sweep_stat, pause_stat);
}

void register_pause(const gc_pause_stat& pause_stat)
{
    gc_instance.register_pause(pause_stat);
}

}}
