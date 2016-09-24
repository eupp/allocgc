#ifndef DIPLOMA_GC_NEW_STATE_H
#define DIPLOMA_GC_NEW_STATE_H

#include <cstddef>
#include <vector>
#include <memory>

#include <boost/range/iterator_range.hpp>

#include <libprecisegc/details/utils/dynarray.hpp>
#include <libprecisegc/details/ptrs/gc_untyped_ptr.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details { namespace threads {

class gc_new_stack : public utils::noncopyable, public utils::nonmovable
{
public:
    typedef std::vector<size_t> offsets_storage_t;
    typedef offsets_storage_t::const_iterator offsets_iterator;
    typedef boost::iterator_range<offsets_iterator> offsets_range;

    class stack_entry
    {
    public:
        stack_entry(byte* ptr, size_t size, bool meta_requested);
        ~stack_entry();
    private:
        friend class gc_new_stack;

        byte*  m_ptr;
        size_t m_size;
        offsets_storage_t m_offsets;
        stack_entry* m_prev;
        bool m_meta_requested;
    };

    gc_new_stack();

    void push_entry(gc_new_stack::stack_entry* entry);
    void pop_entry();

    void register_child(byte* child) const;

    offsets_range offsets() const;

    size_t depth() const noexcept;
    bool is_active() const noexcept;
    bool is_meta_requsted() const noexcept;

    inline bool is_heap_ptr(const byte* ptr) const noexcept
    {
        return m_stack_top && (m_stack_top->m_ptr <= ptr) && (ptr < m_stack_top->m_ptr + m_stack_top->m_size);
    }

    template <typename Functor>
    void trace_pointers(Functor&& f) const
    {
        stack_entry* curr = m_stack_top;
        while (curr) {
            f(curr->m_ptr);
            curr = curr->m_prev;
        }
    }
private:
    static const size_t START_OFFSETS_STORAGE_SIZE = 64;

    gc_new_stack::stack_entry* m_stack_top;
    size_t m_depth;
};

}}}

#endif //DIPLOMA_GC_NEW_STATE_H
