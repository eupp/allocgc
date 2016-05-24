#ifndef DIPLOMA_ROOT_SET_HPP
#define DIPLOMA_ROOT_SET_HPP

#include <libprecisegc/details/gc_untyped_ptr.h>
#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details {

class root_set : private noncopyable
{
public:
    struct element
    {
        gc_untyped_ptr* root;
        element* next;
    };

    root_set();
    root_set(root_set&& other);

    root_set& operator=(root_set&& other);

    ~root_set();

    void add(gc_untyped_ptr* root);
    void remove(gc_untyped_ptr* root);
    bool is_root(gc_untyped_ptr* ptr);

    element* head() const;
private:
    void swap(root_set& other);

    element* m_head;
    element* m_free_list;
};

}}

#endif //DIPLOMA_ROOT_SET_HPP
