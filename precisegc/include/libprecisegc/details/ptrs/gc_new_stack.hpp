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
    typedef utils::dynarray<size_t> offsets_storage_t;
public:
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
        stack_entry(void* new_ptr, size_t new_size);
        ~stack_entry();
    private:
        void* m_old_ptr;
        size_t m_old_size;
        bool m_old_is_meta_requested;
        size_t m_old_offsets_cnt;
        offsets_storage_t m_old_offsets;
    };

    static gc_new_stack& instance();

    void register_child(gc_untyped_ptr* child);

    offsets_range offsets() const;

    size_t depth() const noexcept;

    bool is_active() const noexcept;
    bool is_meta_requsted() const noexcept;
private:
    static const size_t START_OFFSETS_STORAGE_SIZE = 64;

    gc_new_stack();

    void push_offset(size_t offset);

    void* m_top_ptr;
    size_t m_top_size;
    size_t m_depth;
    bool m_is_meta_requested;
    size_t m_top_offsets_cnt;
    offsets_storage_t m_top_offsets;
};

}}}

#endif //DIPLOMA_GC_NEW_STATE_H
