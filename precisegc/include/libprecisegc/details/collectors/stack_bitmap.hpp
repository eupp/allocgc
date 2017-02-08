#ifndef DIPLOMA_APPROX_STACK_MAP_HPP
#define DIPLOMA_APPROX_STACK_MAP_HPP

#include <cmath>
#include <atomic>
#include <bitset>
#include <vector>

#include <boost/integer/static_log2.hpp>

#include <libprecisegc/details/constants.hpp>
#include <libprecisegc/details/gc_handle.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/utils/math.hpp>
#include <libprecisegc/details/gc_interface.hpp>

namespace precisegc { namespace details { namespace collectors {

class stack_bitmap : private utils::noncopyable, private utils::nonmovable
{
public:
    static const size_t STACK_FRAME_SIZE = 512;

    stack_bitmap(byte* stack_start_addr);

    void register_root(gc_handle* root);
    void deregister_root(gc_handle* root);

    size_t size() const;

    void trace(const gc_trace_callback& cb) const;

    inline bool is_stack_ptr(const gc_handle* ptr) const
    {
        const byte* p = reinterpret_cast<const byte*>(ptr);
        return STACK_DIRECTION == stack_growth_direction::UP
               ? (m_stack_start <= p) && (p < m_stack_end)
               : (m_stack_start >= p) && (p > m_stack_end);
    }
private:
    static const size_t STACK_FRAME_MASK = STACK_FRAME_SIZE - 1;
    static const size_t STACK_FRAME_SIZE_LOG2 = boost::static_log2<STACK_FRAME_SIZE>::value;
    static const size_t GC_HANDLE_SIZE = sizeof(gc_handle);
    static const size_t GC_HANDLE_SIZE_LOG2 = boost::static_log2<GC_HANDLE_SIZE>::value;

    static_assert(check_pow2(STACK_FRAME_SIZE), "STACK_FRAME_SIZE is not a power of two");
    static_assert(check_pow2(GC_HANDLE_SIZE), "GC_HANDLE_SIZE is not a power of two");

    bool contains(const gc_handle* ptr) const;

    std::pair<size_t, size_t> root_idxs(const gc_handle* ptr) const;

    typedef std::bitset<STACK_FRAME_SIZE> bitmap_frame_t;

    byte* m_stack_start;
    byte* m_stack_end;
    std::vector<bitmap_frame_t> m_bitmap;
};

}}}

#endif //DIPLOMA_APPROX_STACK_MAP_HPP
