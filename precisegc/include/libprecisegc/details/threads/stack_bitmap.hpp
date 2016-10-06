#ifndef DIPLOMA_APPROX_STACK_MAP_HPP
#define DIPLOMA_APPROX_STACK_MAP_HPP

#include <cmath>
#include <atomic>
#include <bitset>
#include <vector>

#include <boost/integer/static_log2.hpp>

#include <libprecisegc/details/constants.hpp>
#include <libprecisegc/details/gc_word.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/utils/math.hpp>

namespace precisegc { namespace details { namespace threads {

class stack_bitmap : private utils::noncopyable, private utils::nonmovable
{
public:
    static const size_t STACK_FRAME_SIZE = 512;

    stack_bitmap(byte* stack_start_addr);

    void register_root(gc_word* root);
    void deregister_root(gc_word* root);

    bool contains(const gc_word* ptr) const;

    inline bool is_stack_ptr(const gc_word* ptr) const
    {
        const byte* p = reinterpret_cast<const byte*>(ptr);
        return STACK_DIRECTION == stack_growth_direction::UP
                ? (m_stack_start <= p) && (p < m_stack_end)
                : (m_stack_start >= p) && (p > m_stack_end);
    }

    template <typename Functor>
    void trace(Functor&& f) const
    {
        gc_word* it = reinterpret_cast<gc_word*>(m_stack_start);
        for (auto& bitmap_frame: m_bitmap) {
            for (size_t i = 0; i < bitmap_frame.size(); ++i) {
                if (bitmap_frame.test(i)) {
                    f(it);
                }
                STACK_DIRECTION == stack_growth_direction::UP ? ++it : --it;
            }
        }
    }

    size_t count() const;
private:
    static const size_t STACK_FRAME_MASK = STACK_FRAME_SIZE - 1;
    static const size_t STACK_FRAME_SIZE_LOG2 = boost::static_log2<STACK_FRAME_SIZE>::value;
    static const size_t GC_HANDLE_SIZE = sizeof(gc_word);
    static const size_t GC_HANDLE_SIZE_LOG2 = boost::static_log2<GC_HANDLE_SIZE>::value;

    static_assert(check_pow2(STACK_FRAME_SIZE), "STACK_FRAME_SIZE is not a power of two");
    static_assert(check_pow2(GC_HANDLE_SIZE), "GC_HANDLE_SIZE is not a power of two");

    std::pair<size_t, size_t> root_idxs(const gc_word* ptr) const;

    typedef std::bitset<STACK_FRAME_SIZE> bitmap_frame_t;

    byte* m_stack_start;
    byte* m_stack_end;
    std::vector<bitmap_frame_t> m_bitmap;
};

}}}

#endif //DIPLOMA_APPROX_STACK_MAP_HPP
