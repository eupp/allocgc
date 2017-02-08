#ifndef DIPLOMA_GC_NEW_STACK_ENTRY_HPP
#define DIPLOMA_GC_NEW_STACK_ENTRY_HPP

#include <cstddef>
#include <vector>
#include <memory>

#include <boost/range/iterator_range.hpp>

#include <libprecisegc/details/types.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc {

class gc_new_stack_entry : private details::utils::noncopyable, private details::utils::nonmovable
{
public:
    typedef std::vector<size_t> offsets_storage_t;
    typedef offsets_storage_t::const_iterator offsets_iterator;
    typedef boost::iterator_range<offsets_iterator> offsets_range;

    gc_new_stack_entry(details::byte* ptr, size_t size, bool meta_requested);
    ~gc_new_stack_entry();

    details::byte* get_ptr() const;

    gc_new_stack_entry* get_prev() const;
    void set_prev(gc_new_stack_entry* prev);

    void register_offset(size_t offset);

    bool is_meta_requested() const;
private:
    details::byte*  m_ptr;
    size_t m_size;
    offsets_storage_t m_offsets;
    gc_new_stack_entry* m_prev;
    bool m_meta_requested;
};

}

#endif //DIPLOMA_GC_NEW_STACK_ENTRY_HPP
