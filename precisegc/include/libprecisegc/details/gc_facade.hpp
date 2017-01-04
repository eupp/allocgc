#ifndef DIPLOMA_GARBAGE_COLLECTOR_HPP
#define DIPLOMA_GARBAGE_COLLECTOR_HPP

#include <memory>
#include <mutex>

#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/gc_word.hpp>
#include <libprecisegc/details/gc_strategy.hpp>
#include <libprecisegc/details/gc_manager.hpp>
#include <libprecisegc/details/logging.hpp>
#include <libprecisegc/gc_stat.hpp>

namespace precisegc { namespace details {

class gc_facade : private utils::noncopyable, private utils::nonmovable
{
public:
    gc_facade();

    void init(std::unique_ptr<gc_strategy> strategy);

    gc_strategy* get_strategy() const;
    std::unique_ptr<gc_strategy> set_strategy(std::unique_ptr<gc_strategy> strategy);

    allocators::gc_alloc_response allocate(size_t obj_size, size_t obj_cnt, const gc_type_meta* tmeta);

    void commit(gc_cell& cell);
    void commit(gc_cell& cell, const gc_type_meta* type_meta);

    byte* rbarrier(const gc_word& handle);
    void  wbarrier(gc_word& dst, const gc_word& src);

    void interior_wbarrier(gc_word& handle, ptrdiff_t offset);

    byte* pin(const gc_word& handle);
    void  unpin(byte* ptr);

    byte* push_pin(const gc_word& handle);
    void  pop_pin(byte* ptr);

    bool compare(const gc_word& a, const gc_word& b);

    void initiation_point(initiation_point_type ipt, const gc_options& opt);

    bool is_printer_enabled() const;
    void set_printer_enabled(bool enabled);

    void register_page(const byte* page, size_t size);
    void deregister_page(const byte* page, size_t size);

    gc_info  info() const;
    gc_stat  stats() const;
private:
    allocators::gc_alloc_response try_allocate(size_t obj_size, size_t obj_cnt, const gc_type_meta* tmeta);

    static bool is_interior_pointer(const gc_word& handle, byte* iptr);
    static bool is_interior_offset(const gc_word& handle, ptrdiff_t shift);

    std::unique_ptr<gc_strategy> m_strategy;
    gc_manager m_manager;
    std::mutex m_gc_mutex;
};

}}

#endif //DIPLOMA_GARBAGE_COLLECTOR_HPP
