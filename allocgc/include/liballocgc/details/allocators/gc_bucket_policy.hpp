#ifndef ALLOCGC_GC_SO_BUCKET_POLICY_HPP
#define ALLOCGC_GC_SO_BUCKET_POLICY_HPP

#include <cstddef>

#include <boost/math/common_factor_rt.hpp>

#include <liballocgc/gc_common.hpp>
#include <liballocgc/details/constants.hpp>
#include <liballocgc/details/utils/utility.hpp>

namespace allocgc { namespace details { namespace allocators {

class gc_bucket_policy : private utils::noncopyable
{
public:
    class offset_table
    {
    public:
        inline size_t obj_idx(size_t offset) const {
            return m_sz_offset_tbl[offset >> GRANULE_SIZE_LOG2];
        }
    private:
        friend class gc_bucket_policy;

        offset_table(const byte* sz_offset_tbl)
            : m_sz_offset_tbl(sz_offset_tbl)
        {}

        const byte* m_sz_offset_tbl;
    };

    static size_t chunk_size(size_t cell_size)
    {
        static_assert(GC_POOL_CHUNK_OBJECTS_COUNT <= std::numeric_limits<byte>::max(), "chunk size too large");
        size_t sz = boost::math::lcm(cell_size, PAGE_SIZE);
        size_t obj_cnt = sz / cell_size;
        while (obj_cnt < 32) {
            obj_cnt *= 2;
        }
        return obj_cnt * cell_size;
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
            offsets_table_size += chunk_size(sz_cls) / GRANULE_SIZE;
        }

        m_sz_offsets.reset(new byte[offsets_table_size]);

        byte* offset_tbl = m_sz_offsets.get();
        for (size_t i = 0; i < BUCKET_COUNT; ++i) {
            size_t sz_cls = m_sz_cls[i];
            size_t chk_sz = chunk_size(sz_cls);

            assert(sz_cls % GRANULE_SIZE == 0);
            assert(chk_sz % GRANULE_SIZE == 0);

            size_t sz_cls_gran = sz_cls / GRANULE_SIZE;
            size_t chk_sz_gran = chk_sz / GRANULE_SIZE;

            for (size_t j = 0; j < chk_sz_gran; ++j) {
                offset_tbl[j] = j / sz_cls_gran;
            }

            m_sz_offsets_lookup[i] = offset_tbl;
            offset_tbl += chk_sz_gran;
        }
    }

    gc_bucket_policy(gc_bucket_policy&& other)
        : m_sz_cls(other.m_sz_cls)
        , m_sz_tbl(other.m_sz_tbl)
        , m_sz_offsets(std::move(other.m_sz_offsets))
        , m_sz_offsets_lookup(other.m_sz_offsets_lookup)
    {}

    inline size_t bucket_id(size_t size) const {
        return m_sz_tbl[size - 1];
    }

    inline size_t sz_cls(size_t bucket_id) const {
        return m_sz_cls[bucket_id];
    }

    inline offset_table offsets_table(size_t size) const {
        return offset_table(m_sz_offsets_lookup[bucket_id(size)]);
    }

    static const size_t BUCKET_COUNT = 12;
private:
    static const size_t MAX_SIZE = LARGE_CELL_SIZE;

    static_assert(BUCKET_COUNT <= std::numeric_limits<byte>::max(), "Too many buckets");

    // m_sz_cls[i] - size class of i-th bucket
    std::array<size_t, BUCKET_COUNT> m_sz_cls;
    // m_sz_tbl[sz] - number of bucket for size sz
    std::array<byte, MAX_SIZE> m_sz_tbl;
    // memory region for offset tables
    std::unique_ptr<byte[]> m_sz_offsets;
    // m_sz_offsets_lookup[k][i] -
    //      number of object for offset i in memory region that contains objects of k-th size class;
    //      i is offset in a number of granules
    std::array<const byte*, BUCKET_COUNT> m_sz_offsets_lookup;
};

}}}

#endif //ALLOCGC_GC_SO_BUCKET_POLICY_HPP
