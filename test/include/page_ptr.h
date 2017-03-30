#ifndef ALLOCGC_PAGED_DELETER_H
#define ALLOCGC_PAGED_DELETER_H

#include <memory>

#include <liballocgc/details/allocators/gc_core_allocator.hpp>
#include <liballocgc/details/types.hpp>
#include <liballocgc/details/constants.hpp>

struct page_ptr_deleter
{
    void operator()(allocgc::details::byte* mem) const
    {
        using namespace allocgc::details;
        using namespace allocgc::details::allocators;
        gc_core_allocator().deallocate(mem, PAGE_SIZE);
    }
};

typedef std::unique_ptr<allocgc::details::byte, page_ptr_deleter> page_ptr;

page_ptr make_page_ptr()
{
    using namespace allocgc::details;
    using namespace allocgc::details::allocators;
    byte* p = gc_core_allocator().allocate(PAGE_SIZE, false);
    assert(p);
    return page_ptr(p);
}

#endif //ALLOCGC_PAGED_DELETER_H
