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
      LAUNCH_CONCURRENT_MARK
    , COLLECT
};

typedef int gc_gen;

struct gc_options
{
    gc_kind     kind;
    gc_gen      gen;
};

struct gc_memstat
{
    size_t mem_live  = 0;
    size_t mem_used  = 0;
    size_t mem_extra = 0;

    gc_memstat() = default;
    gc_memstat(const gc_memstat&) = default;
    gc_memstat& operator=(const gc_memstat&) = default;

    gc_memstat& operator+=(const gc_memstat& other)
    {
        mem_live  += other.mem_live;
        mem_used  += other.mem_used;
        mem_extra += other.mem_extra;
        return *this;
    }

    friend inline gc_memstat operator+(const gc_memstat& a, const gc_memstat& b)
    {
        return gc_memstat(a) += b;
    }
};

struct gc_stat
{
    size_t              gc_count;
    gc_clock::duration  gc_time;
    gc_memstat          gc_mem;
};

struct gc_collect_stat
{
    size_t mem_freed   = 0;
    size_t mem_moved   = 0;
    size_t pinned_cnt  = 0;

    gc_collect_stat() = default;
    gc_collect_stat(const gc_collect_stat&) = default;
    gc_collect_stat& operator=(const gc_collect_stat&) = default;

    gc_collect_stat& operator+=(const gc_collect_stat& other)
    {
        mem_freed  += other.mem_freed;
        mem_moved  += other.mem_moved;
        pinned_cnt += other.pinned_cnt;
        return *this;
    }

    friend inline gc_collect_stat operator+(const gc_collect_stat& a, const gc_collect_stat& b)
    {
        return gc_collect_stat(a) += b;
    }
};

struct gc_runstat
{
    gc_clock::duration  pause = gc_clock::duration(0);
    gc_collect_stat     collection;
};

enum class gc_loglevel {
      DEBUG
    , INFO
    , WARNING
    , ERROR
    , SILENT
};

}

#endif //ALLOCGC_GC_COMMON_HPP
