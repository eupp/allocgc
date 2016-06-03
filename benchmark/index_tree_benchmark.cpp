#include <nonius/nonius.h++>

#include <vector>
#include <cstdlib>

#include "libprecisegc/details/allocators/index_tree.h"

using namespace precisegc::details;
using namespace precisegc::details::allocators;

typedef int entry_type;
typedef index_tree<entry_type, std::allocator<byte>> index_tree_type;

NONIUS_BENCHMARK("index", [](nonius::chronometer meter)
{
    index_tree_type itree;
    std::vector<entry_type> entries(meter.runs());
    std::vector<byte*> ptrs(meter.runs());
    for (size_t i = 0; i < meter.runs(); ++i) {
        entries[i] = rand();
        ptrs[i] = reinterpret_cast<byte*>(i * PAGE_SIZE);
    }
    meter.measure([&itree, &ptrs, &entries] (size_t i) {
        return itree.index(ptrs[i], PAGE_SIZE, &entries[i]);
    });
});

NONIUS_BENCHMARK("remove_from_index", [](nonius::chronometer meter)
{
    index_tree_type itree;
    std::vector<entry_type> entries(meter.runs());
    std::vector<byte*> ptrs(meter.runs());
    for (size_t i = 0; i < meter.runs(); ++i) {
        entries[i] = rand();
        ptrs[i] = reinterpret_cast<byte*>(i * PAGE_SIZE);
        itree.index(ptrs[i], PAGE_SIZE, &entries[i]);
    }
    meter.measure([&itree, &ptrs, &entries] (size_t i) {
        return itree.remove_index(ptrs[i], PAGE_SIZE);
    });
});

NONIUS_BENCHMARK("get_entry", [](nonius::chronometer meter)
{
    index_tree_type itree;
    std::vector<entry_type> entries(meter.runs());
    std::vector<byte*> ptrs(meter.runs());
    for (size_t i = 0; i < meter.runs(); ++i) {
        entries[i] = rand();
        ptrs[i] = reinterpret_cast<byte*>(i * PAGE_SIZE);
        itree.index(ptrs[i], PAGE_SIZE, &entries[i]);
    }
    meter.measure([&itree, &ptrs] (size_t i) {
        return itree.get_entry(ptrs[i]);
    });
});
