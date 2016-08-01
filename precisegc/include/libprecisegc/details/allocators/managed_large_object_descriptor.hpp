#ifndef DIPLOMA_MANAGED_LARGE_OBJECT_DESCRIPTOR_HPP
#define DIPLOMA_MANAGED_LARGE_OBJECT_DESCRIPTOR_HPP

#include <libprecisegc/details/managed_ptr.hpp>
#include <libprecisegc/details/memory_descriptor.hpp>
#include <libprecisegc/details/constants.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details { namespace allocators {

class managed_large_object_descriptor : public memory_descriptor, private utils::noncopyable, private utils::nonmovable
{
public:
    typedef managed_ptr pointer_type;

    static size_t align_size(size_t size)
    {
        return ((size + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;
    }

    managed_large_object_descriptor(byte* ptr, size_t size);
    ~managed_large_object_descriptor();

    bool get_mark(byte* ptr) const override;
    bool get_pin(byte* ptr) const override;

    void set_mark(byte* ptr, bool mark) override;
    void set_pin(byte* ptr, bool pin) override;

    bool is_live(byte* ptr) const override;
    void set_live(byte* ptr, bool live) override;

    void sweep(byte* ptr) override;

//    virtual void shade(byte* ptr) = 0;

    size_t cell_size() const override;

    object_meta* get_cell_meta(byte* ptr) const override;
    byte* get_cell_begin(byte* ptr) const override;
    byte* get_obj_begin(byte* ptr) const override;

    managed_ptr get_mem();

    bool empty() const;

    memory_descriptor* get_descriptor();
private:
    bool check_ptr(byte* ptr) const;

    byte* m_ptr;
    size_t m_size;
    bool m_live_bit;
    bool m_mark_bit;
    bool m_pin_bit;
};

}}}

#endif //DIPLOMA_MANAGED_LARGE_OBJECT_DESCRIPTOR_HPP
