#ifndef ALLOCGC_GARBAGE_COLLECTOR_HPP
#define ALLOCGC_GARBAGE_COLLECTOR_HPP

#include <memory>
#include <mutex>

#include <liballocgc/details/allocators/memory_index.hpp>
#include <liballocgc/details/utils/utility.hpp>
#include <liballocgc/details/gc_unsafe_scope.hpp>
#include <liballocgc/gc_alloc.hpp>
#include <liballocgc/details/logging.hpp>

namespace allocgc { namespace details {

template <typename GCStrategy>
class gc_facade : private utils::noncopyable, private utils::nonmovable
{
public:
//    static inline void init()
//    {
//        logging::touch();
//        // same hack as with logger
//        allocators::memory_index::init();
//    }

    static inline gc_alloc::response allocate(const gc_alloc::request& rqst)
    {
        return strategy.allocate(rqst);
    }

    static inline void abort(const gc_alloc::response& rsp)
    {
        strategy.abort(rsp);
    }

    static inline void commit(const gc_alloc::response& rsp)
    {
        strategy.commit(rsp);
    }

    static inline void commit(const gc_alloc::response& rsp, const gc_type_meta* type_meta)
    {
        strategy.commit(rsp, type_meta);
    }

    static inline gc_offsets make_offsets(const gc_alloc::response& rsp)
    {
        return strategy.make_offsets(rsp);
    }

    static inline void register_handle(gc_handle& handle, byte* ptr)
    {
        strategy.register_handle(handle, ptr);
    }

    static inline void deregister_handle(gc_handle& handle)
    {
        strategy.deregister_handle(handle);
    }

    static inline byte* register_pin(const gc_handle& handle)
    {
        return strategy.register_pin(handle);
    }

    static inline void deregister_pin(byte* ptr)
    {
        strategy.deregister_pin(ptr);
    }

    static inline byte* push_pin(const gc_handle& handle)
    {
        return strategy.push_pin(handle);
    }

    static inline void pop_pin(byte* ptr)
    {
        strategy.pop_pin(ptr);
    }

    static inline byte* rbarrier(const gc_handle& handle)
    {
        return strategy.rbarrier(handle);
    }

    static inline void wbarrier(gc_handle& dst, const gc_handle& src)
    {
        strategy.wbarrier(dst, src);
    }

    static inline void interior_wbarrier(gc_handle& handle, ptrdiff_t offset)
    {
        strategy.interior_wbarrier(handle, offset);
    }

    static inline bool compare(const gc_handle& a, const gc_handle& b)
    {
        gc_unsafe_scope unsafe_scope;
        return rbarrier(a) == rbarrier(b);
    }

    static inline void register_thread(const thread_descriptor& descr)
    {
        strategy.register_thread(descr);
    }

    static inline void deregister_thread(std::thread::id id)
    {
        strategy.deregister_thread(id);
    }

    static void initiation_point(initiation_point_type ipt, const gc_options& opt)
    {
        strategy.gc(opt);
    }

    static void set_heap_limit(size_t limit)
    {
        strategy.set_heap_limit(limit);
    }

    static inline gc_stat stats()
    {
        return strategy.stats();
    }
private:
    gc_facade() = delete;

    static bool check_gc_kind(gc_kind kind)
    {
        if (kind == gc_kind::CONCURRENT_MARK && !strategy.info().incremental_flag) {
            return false;
        }
        return true;
    }

    static bool is_interior_pointer(const gc_handle& handle, byte* iptr)
    {
        using namespace allocators;

        byte* ptr = rbarrier(handle);
        gc_memory_descriptor* descr = memory_index::get_descriptor(ptr).to_gc_descriptor();
        byte* cell_begin = descr->cell_start(ptr);
        byte* cell_end   = cell_begin + descr->cell_size(ptr);
        return (cell_begin <= iptr) && (iptr < cell_end);
    }

    static bool is_interior_offset(const gc_handle& handle, ptrdiff_t shift)
    {
        return is_interior_pointer(handle, rbarrier(handle) + shift);
    }

    static GCStrategy strategy;
};

template <typename GCStrategy>
GCStrategy gc_facade<GCStrategy>::strategy{};

template <typename GCStrategy>
std::mutex gc_facade<GCStrategy>::gc_mutex{};

}}

#endif //ALLOCGC_GARBAGE_COLLECTOR_HPP
