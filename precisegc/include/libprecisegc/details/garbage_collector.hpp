#ifndef DIPLOMA_GARBAGE_COLLECTOR_HPP
#define DIPLOMA_GARBAGE_COLLECTOR_HPP

#include <memory>
#include <mutex>

#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/gc_handle.hpp>
#include <libprecisegc/details/gc_strategy.hpp>
#include <libprecisegc/details/initiation_policy.hpp>
#include <libprecisegc/details/managed_ptr.hpp>
#include <libprecisegc/details/recorder.hpp>
#include <libprecisegc/details/printer.hpp>
#include <libprecisegc/details/logging.h>
#include <libprecisegc/gc_stat.hpp>

namespace precisegc { namespace details {

class garbage_collector : private utils::noncopyable, private utils::nonmovable
{
public:
    garbage_collector();

    void init(std::unique_ptr<gc_strategy> strategy, std::unique_ptr<initiation_policy> init_policy);

    gc_strategy* get_strategy() const;
    void set_strategy(std::unique_ptr<gc_strategy> strategy);
    std::unique_ptr<gc_strategy> reset_strategy(std::unique_ptr<gc_strategy> strategy);

    managed_ptr allocate(size_t size);

    byte* rbarrier(const gc_handle& handle);
    void  wbarrier(gc_handle& dst, const gc_handle& src);

    void interior_wbarrier(gc_handle& handle, byte* ptr);
    void interior_shift(gc_handle& handle, ptrdiff_t shift);

    byte* pin(const gc_handle& handle);
    void  unpin(byte* ptr);

    bool compare(const gc_handle& a, const gc_handle& b);

    void initiation_point(initiation_point_type ipoint);

    bool is_printer_enabled() const;
    void set_printer_enabled(bool enabled);

    void register_page(const byte* page, size_t size);
    void deregister_page(const byte* page, size_t size);
    void register_pause(const gc_pause_stat& pause_stat);
    void register_sweep(const gc_sweep_stat& sweep_stat, const gc_pause_stat& pause_stat);

    gc_info info() const;
    gc_stat stats() const;
    gc_state state() const;
private:
    bool check_gc_phase(gc_phase phase);

    static bool is_interior_pointer(const gc_handle& handle, byte* p);
    static bool is_interior_shift(const gc_handle& handle, ptrdiff_t shift);

    std::unique_ptr<gc_strategy> m_strategy;
    std::unique_ptr<initiation_policy> m_initiation_policy;
    std::mutex m_gc_mutex;
    recorder m_recorder;
    printer m_printer;
    gc_info m_gc_info;
    bool m_printer_enabled;
};

}}

#endif //DIPLOMA_GARBAGE_COLLECTOR_HPP
