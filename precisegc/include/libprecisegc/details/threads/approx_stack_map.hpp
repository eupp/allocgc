#ifndef DIPLOMA_APPROX_STACK_MAP_HPP
#define DIPLOMA_APPROX_STACK_MAP_HPP

#include <libprecisegc/details/gc_handle.hpp>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details { namespace threads {

class approx_stack_map : private utils::noncopyable, private utils::nonmovable
{
public:
    approx_stack_map();

    void register_root(gc_handle* root);
    void deregister_root(gc_handle* root);

    bool contains(const gc_handle* ptr);

    template <typename Functor>
    void trace(Functor&& f) const
    {}

    size_t count() const;
private:
    static const size_t STACK_FRAME_SIZE = 4;

    class stack_frame
    {
    public:

    private:
        gc_handle* m_data[STACK_FRAME_SIZE];
        size_t m_size;
    };
};

}}}

#endif //DIPLOMA_APPROX_STACK_MAP_HPP
