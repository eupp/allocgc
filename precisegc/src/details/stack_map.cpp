#include <libprecisegc/details/stack_map.hpp>

#include <cassert>
#include <utility>
#include <sys/mman.h>
#include <libprecisegc/details/gc_unsafe_scope.h>

namespace precisegc { namespace details {

stack_map::stack_map()
    : m_head(nullptr)
    , m_free_list(nullptr)
{}

stack_map::stack_map(stack_map&& other)
    : m_head(other.m_head)
    , m_free_list(other.m_free_list)
{}

stack_map::~stack_map()
{
    // We get memory leak here since we are not munmap our memory.
    // It wasn't fixed yet because memory management of stack_map will be refactored soon anyway.
}

stack_map& stack_map::operator=(stack_map&& other)
{
    stack_map(std::move(other)).swap(*this);
    return *this;
}

void stack_map::swap(stack_map& other)
{
    std::swap(m_head, other.m_head);
    std::swap(m_free_list, other.m_free_list);
}

void stack_map::insert(ptrs::gc_untyped_ptr* root)
{
    gc_unsafe_scope unsafe_scope;

    static const size_t PAGE_SIZE = 4096;
    static const size_t ELEM_PER_PAGE = PAGE_SIZE / sizeof(stack_map::element);

    if (m_free_list == nullptr) {
        element * data = (element *) mmap(nullptr, PAGE_SIZE, PROT_WRITE | PROT_READ, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        assert(data != MAP_FAILED);
        for (int i = 0; i < ELEM_PER_PAGE; ++i) {
            data->next = m_free_list;
            m_free_list = data;
            data++;
        }
    }
    element *result = m_free_list;
    m_free_list = m_free_list->next;
    result->next = m_head;
    result->root = root;
    m_head = result;
}

void stack_map::remove(ptrs::gc_untyped_ptr* root)
{
    gc_unsafe_scope unsafe_scope;

    element* prev = nullptr;
    element* curr = m_head;
    while (curr && root != curr->root) {
        prev = curr;
        curr = curr->next;
    }
    assert(curr);

    element* next = curr->next;
    curr->next = m_free_list;
    m_free_list = curr;
    if (prev) {
        prev->next = next;
    } else {
        m_head = next;
    }
}

bool stack_map::contains(ptrs::gc_untyped_ptr* ptr)
{
    element* curr = m_head;
    while (curr && ptr != curr->root) {
        curr = curr->next;
    }
    return curr != nullptr;
}

stack_map::element* stack_map::head() const
{
    return m_head;
}

}}

