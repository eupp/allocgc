#ifndef DIPLOMA_PAGED_DELETER_H
#define DIPLOMA_PAGED_DELETER_H

#include <memory>

#include <libprecisegc/details/allocators/page_allocator.hpp>
#include <libprecisegc/details/types.hpp>
#include <libprecisegc/details/constants.hpp>

struct page_ptr_deleter
{
    void operator()(precisegc::details::byte* mem) const
    {
        using namespace precisegc::details;
        using namespace precisegc::details::allocators;
        page_allocator().deallocate(mem, PAGE_SIZE);
    }
};

typedef std::unique_ptr<precisegc::details::byte, page_ptr_deleter> page_ptr;

page_ptr make_page_ptr()
{
    using namespace precisegc::details;
    using namespace precisegc::details::allocators;
    byte* p = page_allocator().allocate(PAGE_SIZE);
    assert(p);
    return page_ptr(p);
}

#endif //DIPLOMA_PAGED_DELETER_H
