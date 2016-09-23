#ifndef DIPLOMA_APPROX_STACK_MAP_HPP
#define DIPLOMA_APPROX_STACK_MAP_HPP

#include <atomic>
#include <bitset>
#include <cmath>

#include <libprecisegc/details/constants.hpp>
#include <libprecisegc/details/gc_handle.hpp>
#include <libprecisegc/details/allocators/pool.hpp>
#include <libprecisegc/details/utils/dummy_mutex.hpp>
#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/utils/math.hpp>

namespace precisegc { namespace details { namespace threads {

class approx_stack_map : private utils::noncopyable, private utils::nonmovable
{
public:
    static const size_t STACK_FRAME_SIZE = POINTER_BITS_CNT;

    approx_stack_map(byte* stack_start_addr);

    void register_root(gc_handle* root);
    void deregister_root(gc_handle* root);

    bool contains(const gc_handle* ptr) const;

    template <typename Functor>
    void trace(Functor&& f) const
    {
        stack_frame* frame = m_top_frame.load(std::memory_order_relaxed);
        while (frame) {
            frame->trace(std::forward<Functor>(f));
            frame = frame->next();
        }
    }

    size_t count() const;
private:
    static const size_t STACK_FRAME_SIZE_LOG2 = std::log2(POINTER_BITS_CNT);
    static const size_t GC_HANDLE_SIZE = sizeof(gc_handle);
    static const size_t GC_HANDLE_SIZE_LOG2 = std::log2(GC_HANDLE_SIZE);

    static_assert(check_pow2(GC_HANDLE_SIZE), "GC_HANDLE_SIZE is not a power of two");

    static size_t stack_diff_in_words(const gc_handle* ptr, byte* stack_addr);

    class stack_frame : private utils::noncopyable, private utils::nonmovable
    {
    public:
        stack_frame(byte* stack_addr);

        void register_root(gc_handle* root);
        void deregister_root(gc_handle* root);

        inline bool is_upward_than_frame(const gc_handle* ptr) const //__attribute__((always_inline))
        {
            const byte* p = reinterpret_cast<const byte*>(ptr);
            return STACK_DIRECTION == stack_growth_direction::UP
                   ? p >= m_stack_end
                   : p <= m_stack_end;
        }

        inline bool is_upward_than_frame_start(const gc_handle* ptr) const //__attribute__((always_inline))
        {
            const byte* p = reinterpret_cast<const byte*>(ptr);
            return STACK_DIRECTION == stack_growth_direction::UP
                   ? p >= m_stack_begin
                   : p <= m_stack_begin;
        }

        inline bool contains(const gc_handle* ptr) const //__attribute__((always_inline))
        {
            const byte* p = reinterpret_cast<const byte*>(ptr);
            if (STACK_DIRECTION == stack_growth_direction::UP) {
                return (m_stack_begin <= p) && (p < m_stack_end);
            } else {
                return (m_stack_end < p) && (p <= m_stack_begin);
            }
        }

        bool is_registered_root(const gc_handle* ptr) const;
        bool empty() const;

        size_t count() const;

        stack_frame* next() const;
        void set_next(stack_frame* next);

        template <typename Functor>
        void trace(Functor&& f) const
        {
            gc_handle* it = reinterpret_cast<gc_handle*>(m_stack_begin);
            for (size_t i = 0; i < m_bitmap.size(); ++i) {
                if (m_bitmap.test(i)) {
                    f(it);
                }
                STACK_DIRECTION == stack_growth_direction::UP ? ++it : --it;
            }
        }
    private:
        size_t get_root_idx(const gc_handle* root) const;

        byte* m_stack_begin;
        byte* m_stack_end;
        std::bitset<STACK_FRAME_SIZE> m_bitmap;
        std::atomic<stack_frame*> m_next;
    };

    stack_frame* create_frame(const gc_handle* root);

    byte* m_stack_start;
    stack_frame m_stack_bottom;
    std::atomic<stack_frame*> m_top_frame;
    allocators::pool<utils::dummy_mutex> m_pool;
};

}}}

#endif //DIPLOMA_APPROX_STACK_MAP_HPP
