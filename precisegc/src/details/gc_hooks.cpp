#include <libprecisegc/details/gc_hooks.hpp>

#include <cassert>
#include <memory>
#include <iostream>

#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/recorder.hpp>
#include <libprecisegc/details/printer.hpp>

namespace precisegc { namespace details {

class garbage_collector : private utils::noncopyable, private utils::nonmovable
{
public:
    garbage_collector()
        : m_printer(std::clog)
        , m_printer_enabled(false)
    {}

    gc_strategy* get_strategy() const
    {
        return m_strategy.get();
    }

    void set_strategy(std::unique_ptr<gc_strategy> strategy)
    {
        m_strategy = std::move(strategy);
    }

    std::unique_ptr<gc_strategy> reset_strategy(std::unique_ptr<gc_strategy> strategy)
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

    bool is_printer_enabled() const
    {
        return m_printer_enabled;
    }

    void set_printer_enabled(bool enabled)
    {
        m_printer_enabled = enabled;
    }

    void register_pause(const gc_pause_stat& pause_stat)
    {
        m_recorder.register_pause(pause_stat);
        if (m_printer_enabled) {
            m_printer.print_pause_stat(pause_stat);
        }
    }

    void register_sweep(const gc_sweep_stat& sweep_stat, const gc_pause_stat& pause_stat)
    {
        m_recorder.register_sweep(sweep_stat, pause_stat);
        if (m_printer_enabled) {
            m_printer.print_sweep_stat(sweep_stat, pause_stat);
        }
    }

    gc_info info() const
    {
        assert(m_strategy);
        return m_strategy->info();
    }

    gc_stat stat() const
    {
        return m_recorder.stat();
    }
private:
    std::unique_ptr<gc_strategy> m_strategy;
    recorder m_recorder;
    printer m_printer;
    bool m_printer_enabled;
};

static garbage_collector gc_instance;

gc_strategy* gc_get_strategy()
{
    return gc_instance.get_strategy();
}

void gc_set_strategy(std::unique_ptr<gc_strategy> strategy)
{
    gc_instance.set_strategy(std::move(strategy));
}

std::unique_ptr<gc_strategy> gc_reset_strategy(std::unique_ptr<gc_strategy> strategy)
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

gc_info gc_get_info()
{
    return gc_instance.info();
}

gc_stat gc_get_stats()
{
    return gc_instance.stat();
}

void gc_enable_print_stats()
{
    gc_instance.set_printer_enabled(true);
}

void gc_disable_print_stats()
{
    gc_instance.set_printer_enabled(false);
}

void gc_register_pause(const gc_pause_stat& pause_stat)
{
    gc_instance.register_pause(pause_stat);
}

void gc_register_sweep(const gc_sweep_stat& sweep_stat, const gc_pause_stat& pause_stat)
{
    gc_instance.register_sweep(sweep_stat, pause_stat);
}

}}
