#ifndef DIPLOMA_GARBAGE_COLLECTOR_HPP
#define DIPLOMA_GARBAGE_COLLECTOR_HPP

#include <memory>
#include <mutex>

#include <libprecisegc/details/allocators/index_tree.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/gc_strategy.hpp>
#include <libprecisegc/details/gc_manager.hpp>
#include <libprecisegc/details/gc_handle.hpp>
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

    gc_alloc::response allocate(const gc_alloc::request& rqst);

    void abort(const gc_alloc::response& rsp);
    void commit(const gc_alloc::response& rsp);
    void commit(const gc_alloc::response& rsp, const gc_type_meta* type_meta);

    gc_offsets make_offsets(const gc_alloc::response& rsp);

    void register_handle(gc_handle& handle, byte* ptr);
    void deregister_handle(gc_handle& handle);

    byte* register_pin(const gc_handle& handle);
    void  deregister_pin(byte* ptr);

    byte* push_pin(const gc_handle& handle);
    void  pop_pin(byte* ptr);

    byte* rbarrier(const gc_handle& handle);
    void  wbarrier(gc_handle& dst, const gc_handle& src);

    void interior_wbarrier(gc_handle& handle, ptrdiff_t offset);

    bool compare(const gc_handle& a, const gc_handle& b);


    void register_thread(const thread_descriptor& descr);
    void deregister_thread(std::thread::id id);

    bool increment_heap_size(size_t alloc_size);
    void decrement_heap_size(size_t size);
    void set_heap_limit(size_t size);
    void expand_heap();


    void initiation_point(initiation_point_type ipt, const gc_options& opt);

    bool is_printer_enabled() const;
    void set_printer_enabled(bool enabled);

    void register_page(const byte* page, size_t size);
    void deregister_page(const byte* page, size_t size);

    gc_stat stats() const;
private:
    bool is_interior_pointer(const gc_handle& handle, byte* iptr);
    bool is_interior_offset(const gc_handle& handle, ptrdiff_t shift);

    static const size_t HEAP_START_LIMIT;

    static const double INCREASE_FACTOR;
    static const double MARK_THRESHOLD;
    static const double COLLECT_THRESHOLD;

    std::unique_ptr<gc_strategy> m_strategy;
    gc_manager m_manager;
    std::mutex m_gc_mutex;
    std::mutex m_heap_mutex;
    size_t m_heap_limit;
    size_t m_heap_maxlimit;
    size_t m_heap_size;
};

}}

#endif //DIPLOMA_GARBAGE_COLLECTOR_HPP
