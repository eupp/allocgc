#ifndef DIPLOMA_GC_NEW_STATE_H
#define DIPLOMA_GC_NEW_STATE_H

#include <cstddef>
#include <vector>
#include <memory>

#include "util.h"

namespace precisegc { namespace details {

class gc_new_stack : public noncopyable, public nonmovable
{
public:
    class activation_entry
    {
    public:
        activation_entry();
        ~activation_entry();
    };


    class stack_entry
    {
    public:
        stack_entry(void* new_ptr);
        ~stack_entry();
    private:
        std::vector<size_t> m_old_offsets;
        void* m_old_ptr;
    };

    static gc_new_stack& instance();

    std::vector<size_t>& get_top_offsets();
    void* get_top_pointer() const noexcept;

    bool is_active() const noexcept;
private:
    static thread_local std::unique_ptr<gc_new_stack> state_ptr;

    gc_new_stack();

    std::vector<size_t> m_top_offsets;
    void* m_top_ptr;
    size_t m_nesting_level;
};

}}

#endif //DIPLOMA_GC_NEW_STATE_H
