#ifndef DIPLOMA_GC_COMMON_HPP
#define DIPLOMA_GC_COMMON_HPP

#include <thread>
#include <atomic>

#include <boost/range/iterator_range.hpp>

namespace precisegc {

typedef std::uint8_t byte;
typedef std::atomic<byte*> atomic_byte_ptr;

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

//    std::aligned_storage<SIZE> data;
    byte data[SIZE];
};

typedef boost::iterator_range<typename std::vector<size_t>::const_iterator> gc_offsets;

}

#endif //DIPLOMA_GC_COMMON_HPP
