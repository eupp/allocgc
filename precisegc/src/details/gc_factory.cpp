#include <libprecisegc/details/gc_factory.hpp>

#include <libprecisegc/details/utils/make_unique.hpp>
#include <libprecisegc/details/collectors/serial_gc.hpp>
#include <libprecisegc/details/collectors/incremental_gc.hpp>
#include <libprecisegc/details/collectors/initation_policy.hpp>
#include "../../../test/include/incremental_initation_policy_mock.hpp"

namespace precisegc { namespace details {

using namespace collectors;

std::unique_ptr<initation_policy> create_space_based_policy(size_t max_heap_size)
{
    static const size_t start_heap_size     = 2 * 1024 * 1024;  // 2 Mb
    static const double increase_factor     = 2.0;
    static const double threshold           = 1.0;

    return utils::make_unique<space_based_policy>(
              max_heap_size == std::numeric_limits<size_t>::max() ? start_heap_size : max_heap_size
            , threshold
            , increase_factor
            , max_heap_size
    );
}

std::unique_ptr<incremental_initation_policy> create_incremental_space_based_policy(size_t max_heap_size)
{
    static const size_t start_heap_size     = 2 * 1024 * 1024;  // 2 Mb
    static const double increase_factor     = 2.0;
    static const double marking_threshold   = 0.6;
    static const double sweep_threshold     = 1.0;

    return utils::make_unique<incremental_space_based_policy>(
              max_heap_size == std::numeric_limits<size_t>::max() ? start_heap_size : max_heap_size
            , marking_threshold
            , sweep_threshold
            , increase_factor
            , max_heap_size
    );
}

std::unique_ptr<initation_policy> create_initation_policy(gc_init_strategy init_strategy, size_t max_heap_size)
{
    if (init_strategy == gc_init_strategy::MANUAL) {
        return utils::make_unique<empty_policy>();
    } else if (init_strategy == gc_init_strategy::SPACE_BASED || init_strategy == gc_init_strategy::DEFAULT) {
        return create_space_based_policy(max_heap_size);
    }
}

std::unique_ptr<incremental_initation_policy> create_incremental_initation_policy(gc_init_strategy init_strategy, size_t max_heap_size)
{
    if (init_strategy == gc_init_strategy::MANUAL) {
        return utils::make_unique<incremental_empty_policy>();
    } else if (init_strategy == gc_init_strategy::SPACE_BASED || init_strategy == gc_init_strategy::DEFAULT) {
        return create_incremental_space_based_policy(max_heap_size);
    }
}

std::unique_ptr<gc_strategy> gc_factory::create(const gc_options& options)
{
    if (options.type == gc_type::SERIAL) {
        auto init_policy = create_initation_policy(options.init, options.heapsize);
        if (options.compacting == gc_compacting::DISABLED) {
            return utils::make_unique<serial_gc>(std::move(init_policy));
        } else if (options.compacting == gc_compacting::ENABLED) {
            return utils::make_unique<serial_compacting_gc>(std::move(init_policy));
        }
    } else if (options.type == gc_type::INCREMENTAL) {
        auto init_policy = create_incremental_initation_policy(options.init, options.heapsize);
        if (options.compacting == gc_compacting::DISABLED) {
            return utils::make_unique<incremental_gc>(std::move(init_policy));
        } else if (options.compacting == gc_compacting::ENABLED) {
            return utils::make_unique<incremental_compacting_gc>(std::move(init_policy));
        }
    }
}

}}
