#include <libprecisegc/details/root_set.hpp>

#include <cassert>
#include <utility>
#include <sys/mman.h>

namespace precisegc { namespace details {

root_set::root_set()
    : m_head(nullptr)
    , m_free_list(nullptr)
{}

root_set::root_set(root_set&& other)
    : m_head(other.m_head)
    , m_free_list(other.m_free_list)
{}

root_set::~root_set()
{
    // We get memory leak here since we are not munmap our memory.
    // It wasn't fixed yet because memory management of root_set will be refactored soon anyway.
}

root_set& root_set::operator=(root_set&& other)
{
    root_set(std::move(other)).swap(*this);
    return *this;
}

void root_set::swap(root_set& other)
{
    std::swap(m_head, other.m_head);
    std::swap(m_free_list, other.m_free_list);
}

void root_set::add(gc_untyped_ptr* root)
{
    static const size_t PAGE_SIZE = 4096;
    static const size_t ELEM_PER_PAGE = PAGE_SIZE / sizeof(root_set::element);

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

void root_set::remove(gc_untyped_ptr* root)
{
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

bool root_set::is_root(gc_untyped_ptr* ptr)
{
    element* curr = m_head;
    while (curr && ptr != curr->root) {
        curr = curr->next;
    }
    return curr != nullptr;
}

root_set::element* root_set::head() const
{
    return m_head;
}

}}

