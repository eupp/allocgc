#ifndef DIPLOMA_GC_UNTYPED_PIN_H
#define DIPLOMA_GC_UNTYPED_PIN_H

#include <libprecisegc/details/utils/utility.hpp>
#include <libprecisegc/details/gc_handle.hpp>

namespace precisegc { namespace details { namespace ptrs {

class gc_untyped_pin : private utils::noncopyable
{
public:
    gc_untyped_pin(const gc_handle& handle);
    gc_untyped_pin(gc_untyped_pin&& other);
    ~gc_untyped_pin();

    gc_untyped_pin& operator=(gc_untyped_pin&& other);

    void* get() const noexcept;
protected:
    void* m_ptr;
};

}}}

#endif //DIPLOMA_GC_UNTYPED_PIN_H
