#include <nonius/nonius.h++>

#include <vector>

#include <libprecisegc/details/allocators/bitmap_pool_chunk.hpp>
#include <libprecisegc/details/allocators/freelist_pool_chunk.hpp>
#include <libprecisegc/details/allocators/managed_pool_chunk.hpp>

#include "deoptimize.hpp"

using namespace precisegc::details;
using namespace precisegc::details::allocators;

static const size_t OBJ_SIZE = 32;

template <typename Chunk>
void chunk_allocate_benchmark(nonius::chronometer meter)
{
    size_t chunk_size = std::min((size_t) 64, (size_t) Chunk::CHUNK_MAXSIZE);
    size_t chunks_cnt = (meter.runs() + chunk_size - 1) / chunk_size;
    size_t bytes_cnt = OBJ_SIZE * chunk_size;

    std::vector<std::unique_ptr<byte[]>> mem(chunks_cnt);
    std::unique_ptr<byte[]> chunks_storage(new byte[chunks_cnt * sizeof(Chunk)]);
    Chunk* chunks = (Chunk*) chunks_storage.get();

    for (size_t i = 0; i < chunks_cnt; ++i) {
        mem[i].reset(new byte[bytes_cnt]);
        new (chunks + i) Chunk(mem[i].get(), bytes_cnt, OBJ_SIZE);
    }

    meter.measure([&chunks, chunk_size] (size_t i) {
        return chunks[i / chunk_size].allocate(OBJ_SIZE);
    });
}

template <typename Chunk>
void chunk_deallocate_benchmark(nonius::chronometer meter)
{
    std::vector<typename Chunk::pointer_type> ps(meter.runs());

    size_t chunk_size = std::min((size_t) 64, (size_t) Chunk::CHUNK_MAXSIZE);
    size_t chunks_cnt = (meter.runs() + chunk_size - 1) / chunk_size;
    size_t bytes_cnt = OBJ_SIZE * chunk_size;

    std::vector<std::unique_ptr<byte[]>> mem(chunks_cnt);
    std::unique_ptr<byte[]> chunks_storage(new byte[chunks_cnt * sizeof(Chunk)]);
    Chunk* chunks = (Chunk*) chunks_storage.get();

    for (size_t i = 0; i < chunks_cnt; ++i) {
        mem[i].reset(new byte[bytes_cnt]);
        new (chunks + i) Chunk(mem[i].get(), bytes_cnt, OBJ_SIZE);
        for (size_t j = 0; j < chunk_size; ++j) {
            size_t idx = i * chunk_size + j;
            if (idx >= meter.runs()) {
                break;
            }
            ps[idx] = chunks[i].allocate(OBJ_SIZE);
        }
    }

    meter.measure([&ps, &chunks, chunk_size] (size_t i) {
        chunks[i / chunk_size].deallocate(ps[i], OBJ_SIZE);
    });
}

NONIUS_BENCHMARK("pool_chunks.bitmap_pool_chunk.allocate", [](nonius::chronometer meter)
{
    chunk_allocate_benchmark<bitmap_pool_chunk>(meter);
});

NONIUS_BENCHMARK("pool_chunks.bitmap_pool_chunk.deallocate", [](nonius::chronometer meter)
{
    chunk_deallocate_benchmark<bitmap_pool_chunk>(meter);
});

NONIUS_BENCHMARK("pool_chunks.freelist_pool_chunk.allocate", [](nonius::chronometer meter)
{
    chunk_allocate_benchmark<freelist_pool_chunk>(meter);
});

NONIUS_BENCHMARK("pool_chunks.freelist_pool_chunk.deallocate", [](nonius::chronometer meter)
{
    chunk_deallocate_benchmark<freelist_pool_chunk>(meter);
});

NONIUS_BENCHMARK("pool_chunks.managed_pool_chunk.allocate", [](nonius::chronometer meter)
{
    chunk_allocate_benchmark<managed_pool_chunk>(meter);
});

NONIUS_BENCHMARK("pool_chunks.managed_pool_chunk.deallocate", [](nonius::chronometer meter)
{
    chunk_deallocate_benchmark<managed_pool_chunk>(meter);
});