#ifndef DIPLOMA_GC_NEW_STATE_H
#define DIPLOMA_GC_NEW_STATE_H

#include <cstddef>
#include <vector>
#include <memory>

#include "util.h"

namespace precisegc { namespace details {

class gc_new_state: public noncopyable, public nonmovable
{
public:
    class stack_entry
    {
    public:
        stack_entry();
        ~stack_entry();

        void push_new_state(void* new_ptr);
        std::vector<size_t>& get_offsets();
    private:
        std::vector<size_t> m_old_offsets;
        void* m_old_current_ptr;
        bool m_old_is_active;
        bool m_new_state_set;
    };

    static gc_new_state& instance();

    std::vector<size_t>& get_offsets();

    void* get_current_pointer() const noexcept;
    void set_current_pointer(void* ptr) noexcept;

    bool is_active() const noexcept;
    void set_active(bool active) noexcept;
private:
    static thread_local std::unique_ptr<gc_new_state> state_ptr;

    gc_new_state();

    std::vector<size_t> m_offsets;
    void* m_current_ptr;
    bool m_is_active;
};

}}

#endif //DIPLOMA_GC_NEW_STATE_H
