#ifndef DIPLOMA_GC_NEW_STATE_H
#define DIPLOMA_GC_NEW_STATE_H

#include <cstddef>
#include <vector>
#include <memory>

#include <boost/range/iterator_range.hpp>

#include <libprecisegc/details/utils/dynarray.hpp>
#include <libprecisegc/details/ptrs/gc_untyped_ptr.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details { namespace ptrs {

class gc_new_stack : public utils::noncopyable, public utils::nonmovable
{
    class stack_top;
public:
    typedef std::vector<size_t> offsets_storage_t;
    typedef offsets_storage_t::const_iterator offsets_iterator;
    typedef boost::iterator_range<offsets_iterator> offsets_range;

    class activation_entry
    {
    public:
        activation_entry();
        ~activation_entry();
    };

    class stack_entry
    {
    public:
        stack_entry(void* ptr, size_t size);
        ~stack_entry();
    private:
        friend class stack_top;

        void*  m_ptr;
        size_t m_size;
        offsets_storage_t m_offsets;
        stack_entry* m_prev;
    };

    static void register_child(gc_untyped_ptr* child);

    static offsets_range offsets();

    static size_t depth() noexcept;
    static bool is_active() noexcept;
    static bool is_meta_requsted() noexcept;
private:
    class stack_top
    {
    public:
        constexpr stack_top()
            : m_stack_top(nullptr)
            , m_depth(0)
        {}

        void register_child(gc_untyped_ptr* child);

        gc_new_stack::offsets_range offsets() const;

        size_t depth() const noexcept;
        bool is_active() const noexcept;
        bool is_meta_requsted() const noexcept;

        friend class gc_new_stack;
        friend class gc_new_stack::stack_entry;
        friend class gc_new_stack::activation_entry;
    private:
        gc_new_stack::stack_entry* m_stack_top;
        size_t m_depth;
    };

    static const size_t START_OFFSETS_STORAGE_SIZE = 64;

    static thread_local stack_top top;
};

}}}

#endif //DIPLOMA_GC_NEW_STATE_H
