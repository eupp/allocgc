#ifndef ALLOCGC_GC_SO_BUCKET_POLICY_HPP
#define ALLOCGC_GC_SO_BUCKET_POLICY_HPP

#include <cstddef>

#include <liballocgc/gc_common.hpp>
#include <liballocgc/details/constants.hpp>

namespace allocgc { namespace details { namespace allocators {

class gc_bucket_policy
{
public:
    static size_t chunk_size(size_t cell_size)
    {
        static_assert(GC_POOL_CHUNK_OBJECTS_COUNT <= std::numeric_limits<byte>::max(), "chunk size too large");
        return std::min(cell_size * GC_POOL_CHUNK_OBJECTS_COUNT, GC_POOL_CHUNK_MAXSIZE);
    }

    gc_bucket_policy()
        : m_sz_cls{ {32, 48, 64, 96, 128, 192, 256, 384, 512, 1024, 2048, 4096} }
    {
        size_t j = 0;
        size_t offsets_table_size = 0;
        for (size_t i = 0; i < BUCKET_COUNT; ++i) {
            size_t sz_cls = m_sz_cls[i];
            while (j < sz_cls) {
                m_sz_tbl[j++] = i;
            }
            offsets_table_size += chunk_size(sz_cls);
        }

        m_sz_offsets.reset(new byte[offsets_table_size]);

        byte* offset_tbl = m_sz_offsets.get();
        for (size_t i = 0; i < BUCKET_COUNT; ++i) {
            size_t sz_cls = m_sz_cls[i];
            size_t chk_sz = chunk_size(sz_cls);

            for (size_t j = 0; j < chk_sz; ++j) {
                offset_tbl[j] = j / sz_cls;
            }

            m_sz_offsets_lookup[i] = offset_tbl;
            offset_tbl += chk_sz;
        }
    }

    inline size_t bucket_id(size_t size) const {
        return m_sz_tbl[size - 1];
    }

    inline size_t sz_cls(size_t bucket_id) const {
        return m_sz_cls[bucket_id];
    }

    inline const byte* offsets_table(size_t bucket_id) const {
        return m_sz_offsets_lookup[bucket_id];
    }

    static const size_t BUCKET_COUNT = 12;
private:
    static const size_t MAX_SIZE = LARGE_CELL_SIZE;

    static_assert(BUCKET_COUNT <= std::numeric_limits<byte>::max(), "Too many buckets");

    std::array<size_t, BUCKET_COUNT> m_sz_cls;
    std::array<byte, MAX_SIZE> m_sz_tbl;
    std::unique_ptr<byte[]> m_sz_offsets;
    std::array<const byte*, BUCKET_COUNT> m_sz_offsets_lookup;
};

}}}

#endif //ALLOCGC_GC_SO_BUCKET_POLICY_HPP
