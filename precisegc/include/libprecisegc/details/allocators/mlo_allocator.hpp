#ifndef DIPLOMA_MLO_ALLOCATOR_HPP
#define DIPLOMA_MLO_ALLOCATOR_HPP

#include <libprecisegc/details/allocators/managed_object_descriptor.hpp>
#include <libprecisegc/details/allocators/allocator_tag.hpp>

#include <libprecisegc/details/utils/flatten_range.hpp>
#include <libprecisegc/details/utils/locked_range.hpp>
#include <libprecisegc/details/utils/utility.hpp>

#include <libprecisegc/details/compacting/forwarding.hpp>

#include <libprecisegc/details/gc_interface.hpp>
#include <libprecisegc/details/gc_alloc_messaging.hpp>

namespace precisegc { namespace details { namespace allocators {

class mlo_allocator : private utils::noncopyable, private utils::nonmovable
{
    struct control_block
    {
        control_block*   m_next;
        control_block*   m_prev;
    };

    typedef managed_object_descriptor descriptor_t;
public:
    typedef gc_alloc_response pointer_type;
    typedef stateful_alloc_tag alloc_tag;

    mlo_allocator();
    ~mlo_allocator();

    gc_alloc_response allocate(const gc_alloc_request& rqst);

    gc_heap_stat collect(compacting::forwarding& frwd);
    void fix(const compacting::forwarding& frwd);
private:
    static descriptor_t*  get_descriptor(byte* memblk);
    static control_block* get_control_block(byte* memblk);
    static byte*          get_mem(byte* memblk);

    control_block* get_fake_block();

    byte* allocate_block(size_t size);
    void deallocate_block(byte* ptr, size_t size);

    control_block  m_fake;
    control_block* m_head;
};

}}}

#endif //DIPLOMA_MLO_ALLOCATOR_HPP
