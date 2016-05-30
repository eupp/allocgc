#ifndef DIPLOMA_GC_NEW_STATE_H
#define DIPLOMA_GC_NEW_STATE_H

#include <cstddef>
#include <vector>
#include <memory>

#include <libprecisegc/details/utils/utility.hpp>

namespace precisegc { namespace details { namespace ptrs {

class gc_new_stack : public utils::noncopyable, public utils::nonmovable
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
        stack_entry(void* new_ptr, size_t new_size);
        ~stack_entry();
    private:
        std::vector<size_t> m_old_offsets;
        void* m_old_ptr;
        size_t m_old_size;
        bool m_old_is_meta_requested;
    };

    static gc_new_stack& instance();

    std::vector<size_t>& get_top_offsets();
    void* get_top_pointer() const noexcept;
    size_t get_top_size() const noexcept;

    bool is_active() const noexcept;
    bool is_meta_requsted() const noexcept;
private:
    static thread_local std::unique_ptr<gc_new_stack> state_ptr;

    gc_new_stack();

    std::vector<size_t> m_top_offsets;
    void* m_top_ptr;
    size_t m_top_size;
    size_t m_nesting_level;
    bool m_is_meta_requested;
};

}}}

#endif //DIPLOMA_GC_NEW_STATE_H
