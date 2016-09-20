#ifndef DIPLOMA_APPROX_STACK_MAP_HPP
#define DIPLOMA_APPROX_STACK_MAP_HPP

#include <atomic>

#include <libprecisegc/details/gc_handle.hpp>
#include <libprecisegc/details/allocators/pool.hpp>
#include <libprecisegc/details/utils/dummy_mutex.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details { namespace threads {

class approx_stack_map : private utils::noncopyable, private utils::nonmovable
{
public:
    approx_stack_map();

    void register_root(gc_handle* root);
    void deregister_root(gc_handle* root);

    bool contains(const gc_handle* ptr) const;

    template <typename Functor>
    void trace(Functor&& f) const
    {
        stack_frame* frame = m_top.load(std::memory_order_relaxed);
        while (frame) {
            frame->trace(std::forward<Functor>(f));
            frame = frame->next();
        }
    }

    size_t count() const;
private:
    static const size_t STACK_FRAME_SIZE = 4;

    class stack_frame : private utils::noncopyable, private utils::nonmovable
    {
    public:
        stack_frame();

        void push(gc_handle* root);
        bool pop();

        gc_handle* top() const;
        bool contains(const gc_handle* root) const;
        bool contains_strict(const gc_handle* ptr) const;

        bool is_full() const;

        stack_frame* next() const;
        void set_next(stack_frame* next);

        template <typename Functor>
        void trace(Functor&& f) const
        {
            gc_handle* const* begin = m_data;
            gc_handle* const* end = m_data + m_size.load(std::memory_order_relaxed);
            for (auto it = begin; it < end; ++it) {
                f(*it);
            }
        }
    private:
        gc_handle* m_data[STACK_FRAME_SIZE];
        std::atomic<size_t> m_size;
        std::atomic<stack_frame*> m_next;
    };

    void inc_count();
    void dec_count();

    std::atomic<stack_frame*> m_top;
    std::atomic<size_t> m_count;
    allocators::pool<utils::dummy_mutex> m_pool;
};

}}}

#endif //DIPLOMA_APPROX_STACK_MAP_HPP
