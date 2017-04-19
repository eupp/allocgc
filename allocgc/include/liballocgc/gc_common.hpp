#ifndef ALLOCGC_GC_COMMON_HPP
#define ALLOCGC_GC_COMMON_HPP

#include <thread>
#include <atomic>
#include <vector>

#include <boost/range/iterator_range.hpp>

namespace allocgc {

typedef std::uint8_t byte;
typedef std::atomic<byte*> atomic_byte_ptr;

class gc_handle_access;

class gc_handle
{
public:
    gc_handle() = default;

    constexpr gc_handle(byte* ptr)
            : m_ptr(ptr)
    {}

    static gc_handle null;
private:
    friend class gc_handle_access;

    atomic_byte_ptr m_ptr;
};

class gc_handle_access
{
public:
    template<int MemoryOrder>
    static byte* get(const gc_handle& handle)
    {
        return handle.m_ptr.load(static_cast<std::memory_order>(MemoryOrder));
    }

    template<int MemoryOrder>
    static void set(gc_handle& handle, byte* ptr)
    {
        handle.m_ptr.store(ptr, static_cast<std::memory_order>(MemoryOrder));
    }

    template<int MemoryOrder>
    static void advance(gc_handle& handle, ptrdiff_t n)
    {
        handle.m_ptr.fetch_add(n, static_cast<std::memory_order>(MemoryOrder));
    }
};

struct thread_descriptor
{
    std::thread::id id;
    std::thread::native_handle_type native_handle;
    byte* stack_start_addr;
};

struct gc_buf
{
private:
    static const size_t SIZE = 8 * sizeof(size_t);
public:
    static constexpr size_t size()
    {
        return SIZE;
    }

    byte data[SIZE];
};

typedef boost::iterator_range<typename std::vector<size_t>::const_iterator> gc_offsets;

class gc_clock
{
public:
    typedef std::chrono::steady_clock::rep          rep;
    typedef std::chrono::steady_clock::period       period;
    typedef std::chrono::steady_clock::duration     duration;
    typedef std::chrono::steady_clock::time_point   time_point;

    static constexpr bool is_steady = true;

    static time_point now() noexcept
    {
        return std::chrono::steady_clock::now();
    }
};

struct gc_info
{
    bool    incremental_flag;
    bool    support_concurrent_marking;
    bool    support_concurrent_collecting;
};

enum class gc_kind {
      MARK_COLLECT
    , CONCURRENT_MARK
    , COLLECT
    , SKIP
};

typedef int gc_gen;

struct gc_options
{
    gc_kind     kind;
    gc_gen      gen;
};

struct gc_stat
{
    size_t              gc_count;
    gc_clock::duration  gc_time;
};

enum class gc_pause_type {
      MARK_COLLECT
    , TRACE_ROOTS
    , COLLECT
    , SKIP
};

struct gc_pause_stat
{
    gc_pause_type       type      = gc_pause_type::SKIP;
    gc_clock::duration  duration  = gc_clock::duration(0);
};

struct gc_heap_stat
{
    gc_heap_stat() = default;

    gc_heap_stat(const gc_heap_stat&) = default;

    gc_heap_stat& operator=(const gc_heap_stat&) = default;

    gc_heap_stat& operator+=(const gc_heap_stat& other)
    {
        mem_before_gc   += other.mem_before_gc;
        mem_occupied    += other.mem_occupied;
        mem_live        += other.mem_live;
        mem_freed       += other.mem_freed;
        mem_copied      += other.mem_copied;
        pinned_cnt      += other.pinned_cnt;

        return *this;
    }

    friend inline gc_heap_stat operator+(const gc_heap_stat& a, const gc_heap_stat& b)
    {
        return gc_heap_stat(a) += b;
    }

    double residency() const
    {
        return mem_live / (double) mem_occupied;
    }

    size_t mem_before_gc = 0;
    size_t mem_occupied  = 0;
    size_t mem_live      = 0;
    size_t mem_freed     = 0;
    size_t mem_copied    = 0;
    size_t pinned_cnt    = 0;
};

struct gc_run_stat
{
    gc_heap_stat    heap_stat;
    gc_pause_stat   pause_stat;
};

enum class gc_loglevel {
      DEBUG
    , INFO
    , WARNING
    , ERROR
    , SILENT
};

struct gc_params
{
    size_t      heapsize            = std::numeric_limits<size_t>::max();
    size_t      threads_available   = std::thread::hardware_concurrency();
    gc_loglevel loglevel            = gc_loglevel::SILENT;
    bool        conservative        = false;
    bool        compacting          = false;
    bool        incremental         = false;
    bool        manual_init         = false;
    bool        print_stat          = false;
};

}

#endif //ALLOCGC_GC_COMMON_HPP
